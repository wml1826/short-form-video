from __future__ import annotations

import json
import math
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw

CELL_W = 192
CELL_H = 208
COLS = 8
ROWS = 11
ATLAS_W = CELL_W * COLS
ATLAS_H = CELL_H * ROWS

BODY = (206, 155, 123, 255)
OUTLINE = (100, 68, 54, 255)
CHEEK = (250, 217, 135, 255)
MOUTH = (185, 86, 88, 255)
TONGUE = (244, 122, 146, 255)
EYE = (66, 42, 42, 255)
HILITE = (234, 192, 165, 255)


@dataclass
class Pose:
    bob: int = 0
    stretch_x: float = 1.0
    stretch_y: float = 1.0
    lean: float = 0.0
    face_x: float = 0.0
    face_y: float = 0.0
    arm_l: tuple[float, float] = (0.0, 0.0)
    arm_r: tuple[float, float] = (0.0, 0.0)
    leg_l: tuple[float, float] = (0.0, 0.0)
    leg_r: tuple[float, float] = (0.0, 0.0)
    eye_open: float = 1.0
    mouth_open: float = 1.0
    tongue: bool = True
    blush: float = 1.0
    sad: float = 0.0
    wave: float = 0.0
    squish: float = 0.0
    turn: float = 0.0


def ellipse(draw: ImageDraw.ImageDraw, box, fill, outline=OUTLINE, width=3):
    draw.ellipse(box, fill=fill, outline=outline, width=width)


def rounded_poly(draw: ImageDraw.ImageDraw, pts, fill, outline=OUTLINE, width=3):
    draw.polygon(pts, fill=fill, outline=outline)
    if width > 1:
        for i in range(1, len(pts)):
            draw.line((pts[i - 1], pts[i]), fill=outline, width=width)
        draw.line((pts[-1], pts[0]), fill=outline, width=width)


def draw_pet(pose: Pose, look_deg: float | None = None) -> Image.Image:
    img = Image.new("RGBA", (CELL_W, CELL_H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    cx = 96 + int(pose.lean * 10)
    by = 160 + pose.bob
    body_w = int(86 * pose.stretch_x * (1.0 - pose.squish * 0.08))
    body_h = int(108 * pose.stretch_y * (1.0 + pose.squish * 0.06))
    head_w = int(94 * pose.stretch_x)
    head_h = int(84 * pose.stretch_y)
    head_y = by - body_h - 18
    head_x = cx

    body_box = (cx - body_w // 2, by - body_h, cx + body_w // 2, by)
    head_box = (head_x - head_w // 2, head_y, head_x + head_w // 2, head_y + head_h)

    ear_dx = 28
    ear_y = head_y + 8
    ear_size = 18
    left_ear = (head_x - ear_dx - ear_size // 2, ear_y, head_x - ear_dx + ear_size // 2, ear_y + ear_size)
    right_ear = (head_x + ear_dx - ear_size // 2, ear_y, head_x + ear_dx + ear_size // 2, ear_y + ear_size)

    ellipse(d, left_ear, BODY)
    ellipse(d, right_ear, BODY)
    ellipse(d, body_box, BODY)
    ellipse(d, head_box, BODY)

    tuft_x = head_x + int(pose.turn * 4)
    tuft = [(tuft_x - 6, head_y + 4), (tuft_x, head_y - 5), (tuft_x + 6, head_y + 5)]
    rounded_poly(d, tuft, BODY, width=2)

    arm_y = by - body_h + 44
    left_arm = [
        (cx - body_w // 2 + 2, arm_y + 4),
        (cx - body_w // 2 - 18 + int(pose.arm_l[0]), arm_y + 18 + int(pose.arm_l[1])),
        (cx - body_w // 2 - 4 + int(pose.arm_l[0]), arm_y + 34 + int(pose.arm_l[1])),
        (cx - body_w // 2 + 12, arm_y + 18),
    ]
    right_arm_raise = int(-32 * pose.wave)
    right_arm = [
        (cx + body_w // 2 - 2, arm_y + 4),
        (cx + body_w // 2 + 18 + int(pose.arm_r[0]), arm_y + 18 + int(pose.arm_r[1]) + right_arm_raise),
        (cx + body_w // 2 + 2 + int(pose.arm_r[0]), arm_y + 34 + int(pose.arm_r[1]) + right_arm_raise),
        (cx + body_w // 2 - 12, arm_y + 18),
    ]
    rounded_poly(d, left_arm, BODY)
    rounded_poly(d, right_arm, BODY)

    leg_y = by - 10
    leg_gap = 16
    leg_w = 24
    leg_h = 30
    ellipse(
        d,
        (
            cx - leg_gap - leg_w // 2 + int(pose.leg_l[0]),
            leg_y - leg_h + int(pose.leg_l[1]),
            cx - leg_gap + leg_w // 2 + int(pose.leg_l[0]),
            leg_y + int(pose.leg_l[1]),
        ),
        BODY,
    )
    ellipse(
        d,
        (
            cx + leg_gap - leg_w // 2 + int(pose.leg_r[0]),
            leg_y - leg_h + int(pose.leg_r[1]),
            cx + leg_gap + leg_w // 2 + int(pose.leg_r[0]),
            leg_y + int(pose.leg_r[1]),
        ),
        BODY,
    )

    d.ellipse((head_x - 20, head_y + 18, head_x + 14, head_y + 38), fill=HILITE)

    if look_deg is None:
        face_turn_x = pose.face_x
        face_turn_y = pose.face_y
    else:
        rad = math.radians(look_deg)
        face_turn_x = math.sin(rad) * 10
        face_turn_y = -math.cos(rad) * 8

    eye_y = head_y + 38 + int(face_turn_y)
    eye_sep = 15
    eye_w = 6
    eye_h = max(1, int(10 * pose.eye_open))
    left_eye_x = head_x - eye_sep + int(face_turn_x * 0.8)
    right_eye_x = head_x + eye_sep + int(face_turn_x * 0.8)
    d.rounded_rectangle((left_eye_x - eye_w // 2, eye_y - eye_h // 2, left_eye_x + eye_w // 2, eye_y + eye_h // 2), radius=3, fill=EYE)
    d.rounded_rectangle((right_eye_x - eye_w // 2, eye_y - eye_h // 2, right_eye_x + eye_w // 2, eye_y + eye_h // 2), radius=3, fill=EYE)

    cheek_y = head_y + 54 + int(face_turn_y * 0.5)
    cheek_shift = int(face_turn_x * 0.7)
    if pose.blush > 0:
        left_cheek = (head_x - 34 + cheek_shift, cheek_y, head_x - 14 + cheek_shift, cheek_y + 18)
        right_cheek = (head_x + 12 + cheek_shift, cheek_y, head_x + 32 + cheek_shift, cheek_y + 18)
        ellipse(d, left_cheek, CHEEK, width=2)
        ellipse(d, right_cheek, CHEEK, width=2)

    mouth_y = head_y + 54 + int(face_turn_y * 0.8) + int(pose.sad * 4)
    mouth_w = int(14 + 4 * pose.mouth_open)
    mouth_h = int(8 + 4 * pose.mouth_open)
    d.arc((head_x - mouth_w // 2, mouth_y, head_x + mouth_w // 2, mouth_y + mouth_h), 15 if pose.sad < 0.5 else 200, 165 if pose.sad < 0.5 else 340, fill=MOUTH, width=3)
    if pose.tongue and pose.mouth_open > 0.4 and pose.sad < 0.5:
        ellipse(d, (head_x - 4, mouth_y + 5, head_x + 4, mouth_y + 13), TONGUE, outline=MOUTH, width=2)

    return img


def paste_row(atlas: Image.Image, row: int, frames: list[Image.Image]) -> None:
    for col, frame in enumerate(frames):
        atlas.alpha_composite(frame, (col * CELL_W, row * CELL_H))


def seq_idle():
    return [
        Pose(bob=b, eye_open=e, mouth_open=0.8, arm_l=(-2, 0), arm_r=(0, 0))
        for b, e in [(0, 1), (1, 1), (2, 1), (1, 0.2), (0, 1), (-1, 1)]
    ]


def seq_run(direction: int):
    out = []
    offsets = [(-8, 4), (0, -2), (8, 4), (3, 0), (-8, 4), (0, -2), (8, 4), (3, 0)]
    for i, (lean, bob) in enumerate(offsets):
        out.append(
            Pose(
                bob=bob,
                lean=0.35 * direction,
                face_x=6 * direction,
                arm_l=(6 * direction, -4 if i % 2 == 0 else 5),
                arm_r=(-4 * direction, 5 if i % 2 == 0 else -4),
                leg_l=(0, -6 if i % 2 == 0 else 4),
                leg_r=(0, 4 if i % 2 == 0 else -6),
            )
        )
    return out


def seq_wave():
    return [
        Pose(wave=v, arm_l=(-2, 0), face_x=2, mouth_open=1.0)
        for v in [0.0, 0.55, 1.0, 0.45]
    ]


def seq_jump():
    return [
        Pose(bob=v, stretch_y=s, squish=q, arm_l=(0, a), arm_r=(0, a))
        for v, s, q, a in [(0, 0.92, 1.0, 6), (-12, 1.03, 0.2, -4), (-24, 1.08, 0.0, -10), (-10, 1.0, 0.0, -2), (0, 0.95, 0.8, 5)]
    ]


def seq_failed():
    return [
        Pose(bob=b, eye_open=e, mouth_open=0.2, tongue=False, sad=1.0, arm_l=(-2, 8), arm_r=(2, 10), blush=0.4)
        for b, e in [(2, 0.3), (3, 0.2), (4, 0.2), (5, 0.1), (5, 0.1), (4, 0.2), (3, 0.2), (2, 0.3)]
    ]


def seq_waiting():
    return [
        Pose(bob=b, eye_open=1, mouth_open=0.7, face_y=-2, arm_l=(-1, -1), arm_r=(1, -1), wave=w)
        for b, w in [(0, 0), (1, 0), (2, 0.15), (1, 0.15), (0, 0.3), (-1, 0.1)]
    ]


def seq_working():
    return [
        Pose(bob=b, lean=0.1, mouth_open=0.6, face_x=fx, arm_l=(-2, a), arm_r=(2, -a), eye_open=e)
        for b, fx, a, e in [(0, -2, 0, 1), (1, -1, -4, 1), (2, 0, 2, 0.8), (1, 1, -3, 1), (0, 2, 3, 1), (-1, 1, 0, 0.7)]
    ]


def seq_review():
    return [
        Pose(bob=b, lean=l, face_x=fx, mouth_open=0.25, tongue=False, eye_open=e, arm_l=(-2, a))
        for b, l, fx, e, a in [(0, 0.0, -1, 1, 0), (1, -0.08, -2, 1, -1), (1, -0.1, -3, 0.4, -2), (0, -0.12, -2, 1, -2), (-1, -0.05, -1, 1, -1), (0, 0.0, 0, 1, 0)]
    ]


def build_atlas() -> Image.Image:
    atlas = Image.new("RGBA", (ATLAS_W, ATLAS_H), (0, 0, 0, 0))
    rows = [
        seq_idle(),
        seq_run(1),
        seq_run(-1),
        seq_wave(),
        seq_jump(),
        seq_failed(),
        seq_waiting(),
        seq_working(),
        seq_review(),
    ]
    for row_idx, poses in enumerate(rows):
        frames = [draw_pet(pose) for pose in poses]
        paste_row(atlas, row_idx, frames)

    look_angles = [0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5, 180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5]
    for i, angle in enumerate(look_angles):
        frame = draw_pet(Pose(mouth_open=0.7), look_deg=angle)
        atlas.alpha_composite(frame, ((i % 8) * CELL_W, (9 + i // 8) * CELL_H))

    neutral = draw_pet(Pose())
    atlas.alpha_composite(neutral, (6 * CELL_W, 0))
    return atlas


def main():
    out_dir = Path("output/hatch-pet/brownie-pet").resolve()
    package_dir = out_dir / "package"
    final_dir = out_dir / "final"
    final_dir.mkdir(parents=True, exist_ok=True)
    package_dir.mkdir(parents=True, exist_ok=True)

    atlas = build_atlas()
    png_path = final_dir / "spritesheet.png"
    webp_path = package_dir / "spritesheet.webp"
    atlas.save(png_path)
    atlas.save(webp_path, format="WEBP", lossless=True, quality=100, method=6, exact=True)

    pet_json = {
        "id": "brownie-pet",
        "displayName": "Brownie",
        "description": "A soft chubby little coding buddy based on your reference character.",
        "spriteVersionNumber": 2,
        "spritesheetPath": "spritesheet.webp",
    }
    (package_dir / "pet.json").write_text(json.dumps(pet_json, indent=2) + "\n", encoding="utf-8")

    summary = {
        "png": str(png_path),
        "webp": str(webp_path),
        "package": str(package_dir),
    }
    (out_dir / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
