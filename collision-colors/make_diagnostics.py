from pathlib import Path

root = Path(__file__).resolve().parent
diag = root / "diagnostics"
diag.mkdir(exist_ok=True)

lines = (root / "bin" / "collision-colors.bin.pnach").read_text().splitlines()
cut = next(
    (i for i, line in enumerate(lines) if line.startswith("patch=1,EE,204C3358")),
    len(lines),
)
payload = "\n".join(lines[:cut]).rstrip() + "\n\n"
(diag / "payload-marker.pnach").write_text(payload)

for addr in (
    "204C3358",
    "204C33E8",
    "204C349C",
    "204C3560",
    "204C3674",
    "204C3694",
    "205A91E8",
    "205A9240",
    "205A934C",
):
    (diag / f"hook-{addr}.pnach").write_text(
        payload + f"patch=1,EE,{addr},extended,0C03CC00\n"
    )

(diag / "disable-local-tfrag-draw.pnach").write_text(
    "patch=1,EE,20590BD0,extended,03E00008\n"
    "patch=1,EE,20590BD4,extended,00000000\n"
)
