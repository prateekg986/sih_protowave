from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.platypus import (
    BaseDocTemplate,
    Frame,
    Image,
    KeepTogether,
    LongTable,
    PageBreak,
    PageTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "report" / "ProtoWave_Arduino_Technical_Design.pdf"
SCHEMATIC = ROOT / "visuals" / "equipment-placement-schematic.png"
RENDER = ROOT / "visuals" / "bottle-prototype-render-labelled-final.png"

NAVY = colors.HexColor("#103B52")
BLUE = colors.HexColor("#087BBD")
TEAL = colors.HexColor("#3DA2A0")
ORANGE = colors.HexColor("#E56F00")
RED = colors.HexColor("#C94343")
GREEN = colors.HexColor("#26734D")
INK = colors.HexColor("#263943")
MID = colors.HexColor("#5A7380")
PALE = colors.HexColor("#EAF4F8")
PALE_GREEN = colors.HexColor("#EAF6F0")
PALE_ORANGE = colors.HexColor("#FFF2E4")
PALE_RED = colors.HexColor("#FBEAEA")
GRID = colors.HexColor("#B9CDD7")

PAGE_W, PAGE_H = A4
LEFT = 17 * mm
RIGHT = 17 * mm
TOP = 18 * mm
BOTTOM = 16 * mm


def p(text, style):
    return Paragraph(text, style)


def link(label, url):
    return f'<link href="{url}" color="#087BBD">{label}</link>'


def table(data, widths, header=True, font_size=7.6, row_bgs=None):
    t = LongTable(data, colWidths=widths, repeatRows=1 if header else 0, hAlign="LEFT")
    commands = [
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("GRID", (0, 0), (-1, -1), 0.35, GRID),
        ("LEFTPADDING", (0, 0), (-1, -1), 4),
        ("RIGHTPADDING", (0, 0), (-1, -1), 4),
        ("TOPPADDING", (0, 0), (-1, -1), 4),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
        ("FONTNAME", (0, 0), (-1, -1), "Helvetica"),
        ("FONTSIZE", (0, 0), (-1, -1), font_size),
        ("TEXTCOLOR", (0, 0), (-1, -1), INK),
    ]
    if header:
        commands.extend(
            [
                ("BACKGROUND", (0, 0), (-1, 0), NAVY),
                ("TEXTCOLOR", (0, 0), (-1, 0), colors.white),
                ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
            ]
        )
        first_data_row = 1
    else:
        first_data_row = 0
    for row in range(first_data_row, len(data)):
        bg = colors.white if row % 2 else PALE
        commands.append(("BACKGROUND", (0, row), (-1, row), bg))
    if row_bgs:
        for row, bg in row_bgs.items():
            commands.append(("BACKGROUND", (0, row), (-1, row), bg))
    t.setStyle(TableStyle(commands))
    return t


styles = getSampleStyleSheet()
styles.add(
    ParagraphStyle(
        "CoverTitle",
        parent=styles["Title"],
        fontName="Helvetica-Bold",
        fontSize=27,
        leading=30,
        textColor=NAVY,
        alignment=TA_LEFT,
        spaceAfter=8,
    )
)
styles.add(
    ParagraphStyle(
        "CoverSub",
        parent=styles["Normal"],
        fontName="Helvetica",
        fontSize=12.5,
        leading=17,
        textColor=MID,
        spaceAfter=10,
    )
)
styles.add(
    ParagraphStyle(
        "H1x",
        parent=styles["Heading1"],
        fontName="Helvetica-Bold",
        fontSize=19,
        leading=23,
        textColor=NAVY,
        spaceBefore=2,
        spaceAfter=8,
    )
)
styles.add(
    ParagraphStyle(
        "H2x",
        parent=styles["Heading2"],
        fontName="Helvetica-Bold",
        fontSize=12.5,
        leading=15,
        textColor=BLUE,
        spaceBefore=8,
        spaceAfter=4,
    )
)
styles.add(
    ParagraphStyle(
        "Bodyx",
        parent=styles["BodyText"],
        fontName="Helvetica",
        fontSize=9.2,
        leading=13.2,
        textColor=INK,
        spaceAfter=6,
    )
)
styles.add(
    ParagraphStyle(
        "Smallx",
        parent=styles["BodyText"],
        fontName="Helvetica",
        fontSize=7.6,
        leading=10.2,
        textColor=MID,
        spaceAfter=3,
    )
)
styles.add(
    ParagraphStyle(
        "Bulletx",
        parent=styles["BodyText"],
        fontName="Helvetica",
        fontSize=9,
        leading=12.6,
        leftIndent=12,
        firstLineIndent=-8,
        bulletIndent=0,
        textColor=INK,
        spaceAfter=3,
    )
)
styles.add(
    ParagraphStyle(
        "CalloutTitle",
        parent=styles["Heading2"],
        fontName="Helvetica-Bold",
        fontSize=11.5,
        leading=14,
        textColor=NAVY,
        spaceAfter=3,
    )
)
styles.add(
    ParagraphStyle(
        "TableText",
        parent=styles["BodyText"],
        fontName="Helvetica",
        fontSize=7.4,
        leading=9.3,
        textColor=INK,
    )
)
styles.add(
    ParagraphStyle(
        "TableHead",
        parent=styles["BodyText"],
        fontName="Helvetica-Bold",
        fontSize=7.3,
        leading=9.2,
        textColor=colors.white,
    )
)
styles.add(
    ParagraphStyle(
        "Quote",
        parent=styles["BodyText"],
        fontName="Helvetica-Bold",
        fontSize=12,
        leading=16,
        textColor=NAVY,
        alignment=TA_CENTER,
    )
)


def header_footer(canvas, doc):
    canvas.saveState()
    canvas.setStrokeColor(GRID)
    canvas.setLineWidth(0.5)
    canvas.line(LEFT, PAGE_H - 12 * mm, PAGE_W - RIGHT, PAGE_H - 12 * mm)
    canvas.setFont("Helvetica-Bold", 7.5)
    canvas.setFillColor(NAVY)
    canvas.drawString(LEFT, PAGE_H - 9 * mm, "PROTOWAVE / ARDUINO MEGA ENGINEERING DESIGN")
    canvas.setFont("Helvetica", 7.5)
    canvas.setFillColor(MID)
    canvas.drawRightString(PAGE_W - RIGHT, 8 * mm, f"SIH26040 technical package  |  {doc.page}")
    canvas.restoreState()


doc = BaseDocTemplate(
    str(OUT),
    pagesize=A4,
    leftMargin=LEFT,
    rightMargin=RIGHT,
    topMargin=TOP,
    bottomMargin=BOTTOM,
    title="ProtoWave Arduino Mega Technical Design",
    author="ProtoWave project team",
    subject="Gravity-fed bottle prototype, sensor placement, thresholds and firmware",
)
frame = Frame(LEFT, BOTTOM, PAGE_W - LEFT - RIGHT, PAGE_H - TOP - BOTTOM, id="body")
doc.addPageTemplates([PageTemplate(id="main", frames=[frame], onPage=header_footer)])

story = []

# Cover
story += [
    Spacer(1, 9 * mm),
    p("ProtoWave", styles["CoverTitle"]),
    p("Arduino Mega gravity-fed water purification and quality interlock", styles["CoverSub"]),
    p("Sensor placement, stepped-bottle hydraulics, evidence-based thresholds, safe wiring, firmware and validation plan", styles["CoverSub"]),
]
cover_box = Table(
    [[
        p("DESIGN POSITION", styles["CalloutTitle"]),
        p("Build a <b>descending, modular treatment train</b>. Use the listed sensors for process control, not as proof that fluoride or heavy metals are absent. Keep the drinking-water tap fail-closed until calibration and validation are complete.", styles["Bodyx"]),
    ]],
    colWidths=[37 * mm, 132 * mm],
)
cover_box.setStyle(TableStyle([
    ("BACKGROUND", (0, 0), (-1, -1), PALE_GREEN),
    ("BOX", (0, 0), (-1, -1), 1.2, TEAL),
    ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
    ("LEFTPADDING", (0, 0), (-1, -1), 8),
    ("RIGHTPADDING", (0, 0), (-1, -1), 8),
    ("TOPPADDING", (0, 0), (-1, -1), 8),
    ("BOTTOMPADDING", (0, 0), (-1, -1), 8),
]))
story += [cover_box, Spacer(1, 8 * mm)]
img = Image(str(RENDER), width=176 * mm, height=98.8 * mm)
story += [img, Spacer(1, 5 * mm)]
story += [
    p("Prototype for Smart India Hackathon 2026 problem statement SIH26040", styles["Smallx"]),
    p("Design revision: Arduino Mega 2560 | Fail-closed control | Gravity service and separate reverse-flow backwash", styles["Smallx"]),
    PageBreak(),
]

# Executive decision
story += [p("1. Executive engineering decision", styles["H1x"])]
decision_rows = [
    [p("Decision", styles["TableHead"]), p("Recommended implementation", styles["TableHead"]), p("Reason", styles["TableHead"])],
    [p("Controller", styles["TableText"]), p("Arduino Mega 2560", styles["TableText"]), p("16 analog inputs and four hardware serial ports leave enough I/O for sensors, valves and SIM800L without software serial.", styles["TableText"])],
    [p("Hydraulics", styles["TableText"]), p("Descending bottle staircase", styles["TableText"]), p("Preserves available static head, avoids uphill loops and reduces trapped air at low pressure.", styles["TableText"])],
    [p("Treatment", styles["TableText"]), p("Source-specific, replaceable cartridges", styles["TableText"]), p("High-fluoride groundwater and acid mine drainage require different chemistry; limestone must be bypassable.", styles["TableText"])],
    [p("Sensors", styles["TableText"]), p("One final online sensor cell before UV", styles["TableText"]), p("Measures the quality actually presented to the disinfection barrier and tap.", styles["TableText"])],
    [p("Safety", styles["TableText"]), p("Normally-closed tap plus software interlocks", styles["TableText"]), p("Loss of power, invalid calibration or unsafe measurements closes the drinking-water path.", styles["TableText"])],
    [p("Fluoride", styles["TableText"]), p("Optional ISE/colorimetric side-stream", styles["TableText"]), p("pH/TDS/turbidity do not directly measure fluoride. Reagent-conditioned sample must drain to waste.", styles["TableText"])],
    [p("Metals", styles["TableText"]), p("Accredited lab release testing; optional validated ASV R&D module", styles["TableText"]), p("The listed sensor set cannot identify As, Pb, Cd, Cr, Ni, Hg, Mn, Cu or Fe.", styles["TableText"])],
]
story += [table(decision_rows, [28 * mm, 61 * mm, 80 * mm], font_size=7.4), Spacer(1, 5 * mm)]
story += [p("Blunt corrections to the original concept", styles["H2x"])]
for text in [
    "A single motorized valve cannot reverse flow. True gravity backwash needs separate feed, tap, clean-backwash and waste paths, or a pump.",
    "A generic 12 V UV-C LED chamber is not automatically a disinfection barrier. Delivered dose, water transmittance, flow and lamp/intensity status must be validated.",
    "Automatic tap locking is defensible only for parameters that are actually measured and for sensor failures. Do not label a TDS excursion as fluoride or metal toxicity.",
    "Transparent PET is useful for a short judging demonstration. A deployable unit needs opaque, cleanable, UV-stable and pressure-rated housings.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [Spacer(1, 3 * mm)]
truth = Table([[p("The award-winning version is the one that clearly separates <b>what is measured</b>, <b>what is inferred</b>, and <b>what is laboratory-verified</b>.", styles["Quote"])]], colWidths=[169 * mm])
truth.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), PALE_ORANGE), ("BOX", (0, 0), (-1, -1), 1, ORANGE), ("PADDING", (0, 0), (-1, -1), 10)]))
story += [truth, PageBreak()]

# Gravity
story += [p("2. Why the filters should step downward", styles["H1x"])]
story += [p("Yes - the bottle prototype should descend from the raw-water reservoir to the final tap. The staircase is not a series of pumps and it does not multiply pressure. It protects the single pressure source already available: elevation.", styles["Bodyx"])]
formula = Table([[p("P = rho g h", styles["Quote"]), p("For water: 1.0 m head is approximately 9.8 kPa or 0.098 bar.", styles["Bodyx"])]], colWidths=[52 * mm, 117 * mm])
formula.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), PALE), ("BOX", (0, 0), (-1, -1), 1, BLUE), ("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("PADDING", (0, 0), (-1, -1), 8)]))
story += [formula, Spacer(1, 4 * mm)]
gravity_rows = [
    [p("Layout", styles["TableHead"]), p("Will it flow?", styles["TableHead"]), p("Engineering consequence", styles["TableHead"])],
    [p("Descending staircase", styles["TableText"]), p("Best passive option", styles["TableText"]), p("Every inter-stage connection can stay level or fall. Venting is simple; bubbles rise out rather than collecting at high points.", styles["TableText"])],
    [p("Same-height row", styles["TableText"]), p("Can work", styles["TableText"]), p("Only if the raw surface remains above the tap, tubing is fully primed and pressure losses stay below the available head.", styles["TableText"])],
    [p("Tubing rises between stages", styles["TableText"]), p("Unreliable at bottle scale", styles["TableText"]), p("Creates high points that trap air and consume head; flow may stop as media loads.", styles["TableText"])],
]
story += [table(gravity_rows, [38 * mm, 37 * mm, 94 * mm]), Spacer(1, 4 * mm)]
for text in [
    "Set the raw-water surface at least 1.0-1.5 m above the tap for the prototype; confirm by measurement rather than relying on the drawing.",
    "Give each bottle a vented top inlet and a bottom outlet. Use short, large-bore food-grade tubing and avoid sharp elbows.",
    "A 100-150 mm downward step between bottle shelves is practical. Total raw-surface-to-tap height matters more than identical step height.",
    "Measure flow at high and low tank levels, then with partly clogged media. The lowest measured head is the design case.",
    "Backwash is different: place a clean-water tank above every filter and send clean water upward through the selected bed to a separate waste bottle.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [Spacer(1, 4 * mm), Image(str(SCHEMATIC), width=176 * mm, height=112.6 * mm), p("Exact equipment and sensor placement. Blue is service flow; orange dashed is the separate backwash path.", styles["Smallx"]), PageBreak()]

# Treatment train
story += [p("3. Bottle train and construction", styles["H1x"])]
train_rows = [
    [p("Stage", styles["TableHead"]), p("Prototype construction", styles["TableHead"]), p("Purpose and design caution", styles["TableHead"])],
    [p("0 Raw reservoir", styles["TableText"]), p("Vented 5 L food-grade bottle, highest shelf; outlet above settled sludge; S0 raw sample port.", styles["TableText"]), p("Provides static head. Add a low-level float switch before unattended operation.", styles["TableText"])],
    [p("1 Solids", styles["TableText"]), p("Top: washed coarse gravel. Middle: graded quartz sand. Bottom: support gravel and nylon retaining mesh.", styles["TableText"]), p("Reduces particles before sorbents and UV. Wash until rinse water is clear; prevent media escape.", styles["TableText"]),],
    [p("2 pH conditioning", styles["TableText"]), p("High-calcium limestone cartridge with a hydraulic bypass.", styles["TableText"]), p("Use for acidic mine drainage. Bypass if raw water is already neutral/alkaline; excessive pH can damage downstream performance.", styles["TableText"])],
    [p("3 Metal polish", styles["TableText"]), p("Separate, replaceable natural-zeolite and characterized biochar layers/cartridges.", styles["TableText"]), p("Potential polishing only. Metals removal is source-dependent and requires breakthrough testing plus lab confirmation.", styles["TableText"]),],
    [p("4 Fluoride", styles["TableText"]), p("Activated alumina baseline, or validated calcium-modified biochar/sorbent.", styles["TableText"]), p("Do not claim a fixed capacity from literature. Competing ions, pH and contact time control breakthrough.", styles["TableText"]),],
    [p("5 Sensor cell", styles["TableText"]), p("Opaque flow-through cell; pH, TDS and turbidity probes; drain and calibration ports.", styles["TableText"]), p("Place after all media and before UV. Exclude sunlight, bubbles and electrical cross-talk.", styles["TableText"]),],
    [p("6 Barrier/outlet", styles["TableText"]), p("Low-flow meter, validated stainless UV-C reactor, normally-closed tap solenoid.", styles["TableText"]), p("No unmonitored treatment component between sensor cell and UV. Keep UV inaccessible and interlocked.", styles["TableText"]),],
]
story += [table(train_rows, [25 * mm, 72 * mm, 72 * mm], font_size=7.25), Spacer(1, 5 * mm)]
story += [p("Scale warning", styles["H2x"])]
story += [p("A nominal 2 L bottle may hold only about 1.2-1.5 L of packed media. At 0.25 L/min, empty-bed contact time is roughly 5-6 minutes; at 1 L/min it is only about 1-1.5 minutes. The YF-S201 is a poor match for the slower, more chemically credible bottle flow. For deployment, the proposed 3-4 inch diameter, 2 m columns provide a much larger bed volume, but their contact time and capacity still require pilot data.", styles["Bodyx"])]
story += [p("Two source-specific operating modes", styles["H2x"])]
mode_rows = [
    [p("Mode", styles["TableHead"]), p("Recommended route", styles["TableHead"]), p("Release evidence", styles["TableHead"])],
    [p("F: high-fluoride groundwater", styles["TableText"]), p("Solids -> limestone bypassed unless needed -> fluoride sorbent -> polish -> sensor cell -> UV.", styles["TableText"]), p("Fluoride breakthrough curve, pH/TDS/turbidity, metals panel, E. coli/coliform and validated UV dose.", styles["TableText"]),],
    [p("M: acidic/mine-affected", styles["TableText"]), p("Solids -> limestone -> oxidation/settling if needed -> zeolite/biochar -> fluoride cartridge only if fluoride is confirmed -> sensors -> UV.", styles["TableText"]), p("Full dissolved/total metals panel, pH/alkalinity, sulfate, TDS, turbidity, microbial panel and UV validation.", styles["TableText"]),],
]
story += [table(mode_rows, [42 * mm, 72 * mm, 55 * mm], font_size=7.25), PageBreak()]

# Sensor placement
story += [p("4. Exact sensor placement", styles["H1x"])]
sensor_rows = [
    [p("Device", styles["TableHead"]), p("Place it here", styles["TableHead"]), p("Installation detail", styles["TableHead"]), p("Why", styles["TableHead"])],
    [p("pH probe", styles["TableText"]), p("First position in final sensor cell, after all media and before UV.", styles["TableText"]), p("Vertical or within manufacturer angle; glass bulb continuously wet; low-velocity bypass pocket; calibration access.", styles["TableText"]), p("Controls limestone bypass/chemical acceptability and sees final treated chemistry.", styles["TableText"]),],
    [p("TDS probe", styles["TableText"]), p("Same cell downstream of pH, physically separated.", styles["TableText"]), p("No bubbles; add temperature probe; isolate or time-multiplex excitation if probes interfere.", styles["TableText"]), p("Tracks dissolved-ion change. It cannot identify fluoride or metals.", styles["TableText"]),],
    [p("Turbidity", styles["TableText"]), p("Last probe in opaque cell, immediately before flow meter/UV.", styles["TableText"]), p("Shield from ambient light; use bubble trap; avoid deposits on optical windows; provide zero/standard port.", styles["TableText"]), p("Interlocks UV when particles may shield microorganisms.", styles["TableText"]),],
    [p("Flow meter", styles["TableText"]), p("After sensor cell and before UV.", styles["TableText"]), p("Straight pipe per sensor requirement; arrow with flow; calibrate at actual gravity head.", styles["TableText"]), p("Dose and clog monitoring. Downstream placement measures water actually reaching UV.", styles["TableText"]),],
    [p("Voltage", styles["TableText"]), p("Inside dry IP65 control panel across fused 12 V bus.", styles["TableText"]), p("Never contact water; calibrated divider; shared reference or isolation as designed.", styles["TableText"]), p("Detects low battery/brownout and protects fail-closed operation.", styles["TableText"]),],
    [p("Optional fluoride ISE", styles["TableText"]), p("Post-filter side-stream after fluoride stage; sample then drains to waste.", styles["TableText"]), p("High-impedance amplifier; add TISAB to sample only; standards bracket 0.1-20 mg/L; temperature compensated.", styles["TableText"]), p("Provides direct fluoride evidence. Reagent must never enter product water.", styles["TableText"]),],
    [p("Raw/sample ports", styles["TableText"]), p("S0 raw, then post-stage ports after stages 1-4.", styles["TableText"]), p("Capped, disinfectable fittings; collect without contaminating product path.", styles["TableText"]), p("Makes media failure and breakthrough diagnosable instead of guessing.", styles["TableText"]),],
]
story += [table(sensor_rows, [24 * mm, 45 * mm, 57 * mm, 43 * mm], font_size=6.9), Spacer(1, 5 * mm)]
story += [p("Mechanical rules for reliable readings", styles["H2x"])]
for text in [
    "Put the sensing cell downstream of the last media stage. A sensor before treatment is useful for research, but it cannot interlock the product tap by itself.",
    "Keep the turbidity optical path continuously flooded and free of bubbles. Ambient light and air bubbles can look like unsafe turbidity.",
    "Do not place the probes in the raw reservoir: stagnant gradients and settled solids will make readings unrepresentative.",
    "Keep mains/valve wires away from high-impedance pH and fluoride signals; use shielded cable, strain relief and star grounding.",
    "Add two level switches for the next revision: source-water-present and backwash-water-ready. Without them, automatic backwash can run dry.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [PageBreak()]

# Thresholds
story += [p("5. Detection levels and controller thresholds", styles["H1x"])]
story += [p("A standard limit, a sensor detection/quantification level and a controller set-point are different. The limits below are grounded in the Jal Jeevan Mission/IS 10500 table and WHO guidance; the guard bands are conservative project choices.", styles["Bodyx"])]
threshold_rows = [
    [p("Parameter", styles["TableHead"]), p("Reference value", styles["TableHead"]), p("Warn", styles["TableHead"]), p("Lock/action", styles["TableHead"]), p("Required capability and verdict", styles["TableHead"])],
    [p("pH", styles["TableText"]), p("6.5-8.5; no relaxation", styles["TableText"]), p("<6.7 or >8.3", styles["TableText"]), p("<6.5 or >8.5; immediate <5 or >10", styles["TableText"]), p("Verified error <=0.1 pH. PH-4502C is acceptable only as a calibrated prototype input.", styles["TableText"]),],
    [p("Turbidity", styles["TableText"]), p("1 NTU acceptable; 5 NTU permissible", styles["TableText"]), p(">=0.8 NTU", styles["TableText"]), p(">=1.0 NTU; immediate >=5", styles["TableText"]), p("Need low-range 0-10 NTU measurement with <=0.1 NTU resolution and validated uncertainty. TS-300B is trend-only.", styles["TableText"]),],
    [p("TDS", styles["TableText"]), p("500 mg/L acceptable; 2000 conditional", styles["TableText"]), p(">500 mg/L", styles["TableText"]), p(">=900 mg/L with listed board: instrument ceiling/unknown", styles["TableText"]), p("Board range 0-1000 ppm and about +/-10% full scale cannot verify 2000. TDS is non-specific.", styles["TableText"]),],
    [p("Fluoride", styles["TableText"]), p("1.0 mg/L acceptable; 1.5 permissible/WHO", styles["TableText"]), p(">=1.0 mg/L", styles["TableText"]), p(">=1.4 mg/L; immediate >=2.0", styles["TableText"]), p("Direct method LOQ <=0.1 mg/L over at least 0.1-20 mg/L. Not measured by listed sensors.", styles["TableText"]),],
    [p("Flow", styles["TableText"]), p("Process baseline", styles["TableText"]), p("<80% clean-bed flow", styles["TableText"]), p("Backwash if <60% for 30 s during a request", styles["TableText"]), p("YF-S201 is normally about 1-30 L/min. Use a calibrated 0.3-6 L/min device if the rig is slower.", styles["TableText"]),],
    [p("12 V bus", styles["TableText"]), p("Engineering value", styles["TableText"]), p("<11.2 V", styles["TableText"]), p("<10.8 V", styles["TableText"]), p("Calibrate divider and separately verify Mega brownout behavior under valve/SIM800L load.", styles["TableText"]),],
]
story += [table(threshold_rows, [22 * mm, 38 * mm, 28 * mm, 41 * mm, 40 * mm], font_size=6.7), Spacer(1, 5 * mm)]
note = Table([[p("TDS policy", styles["CalloutTitle"]), p("500 mg/L is the acceptable value; 2000 mg/L is conditional when no alternative source exists. The firmware warns at 500 but locks near 900 because the listed sensor cannot measure the conditional limit. A public unit needs a validated wider-range conductivity instrument and an explicit local release policy.", styles["Bodyx"])]], colWidths=[30 * mm, 139 * mm])
note.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), PALE_ORANGE), ("BOX", (0, 0), (-1, -1), 1, ORANGE), ("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("PADDING", (0, 0), (-1, -1), 7)]))
story += [note, Spacer(1, 5 * mm)]
story += [p("Metals release criteria", styles["H2x"])]
metal_rows = [
    [p("Analyte", styles["TableHead"]), p("Acceptable / permissible (mg/L)", styles["TableHead"]), p("Project LOQ goal (mg/L)", styles["TableHead"]), p("Status", styles["TableHead"])],
    [p("Arsenic", styles["TableText"]), p("0.01 / 0.05", styles["TableText"]), p("<=0.001", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Lead", styles["TableText"]), p("0.01 / no relaxation", styles["TableText"]), p("<=0.001", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Cadmium", styles["TableText"]), p("0.003 / no relaxation", styles["TableText"]), p("<=0.0003", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Total chromium", styles["TableText"]), p("0.05 / no relaxation", styles["TableText"]), p("<=0.005", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Nickel", styles["TableText"]), p("0.02 / no relaxation", styles["TableText"]), p("<=0.002", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Mercury", styles["TableText"]), p("0.001 / no relaxation", styles["TableText"]), p("<=0.0001", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Manganese", styles["TableText"]), p("0.1 / 0.3", styles["TableText"]), p("<=0.01", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Copper", styles["TableText"]), p("0.05 / 1.5", styles["TableText"]), p("<=0.005", styles["TableText"]), p("Laboratory only", styles["TableText"])],
    [p("Iron", styles["TableText"]), p("1.0 / no relaxation", styles["TableText"]), p("<=0.1", styles["TableText"]), p("Laboratory only", styles["TableText"])],
]
story += [table(metal_rows, [35 * mm, 54 * mm, 42 * mm, 38 * mm], font_size=7.1), p("LOQ goals are project QA targets near one-tenth of the applicable limit, not statutory values.", styles["Smallx"]), PageBreak()]

# Backwash
story += [p("6. Service flow and real gravity backwash", styles["H1x"])]
story += [p("Normal service is top-to-bottom through each vented bottle. Backwash must reverse through the selected bed from bottom to top. A separate elevated clean-water tank supplies the reverse-flow head; dirty water is isolated from the product line and sent to waste.", styles["Bodyx"])]
valve_rows = [
    [p("State", styles["TableHead"]), p("V1 feed", styles["TableHead"]), p("V2 NC tap", styles["TableHead"]), p("V3 clean backwash", styles["TableHead"]), p("V4 waste", styles["TableHead"]), p("UV", styles["TableHead"])],
    [p("Locked/idle", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Off", styles["TableText"])],
    [p("Ready", styles["TableText"]), p("Open", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Off", styles["TableText"])],
    [p("UV warm-up", styles["TableText"]), p("Open", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("On", styles["TableText"])],
    [p("Dispense", styles["TableText"]), p("Open", styles["TableText"]), p("Open", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("On", styles["TableText"])],
    [p("Isolate/settle", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Off", styles["TableText"])],
    [p("Reverse backwash", styles["TableText"]), p("Closed", styles["TableText"]), p("Closed", styles["TableText"]), p("Open", styles["TableText"]), p("Open", styles["TableText"]), p("Off", styles["TableText"])],
]
story += [table(valve_rows, [40 * mm, 25 * mm, 27 * mm, 35 * mm, 22 * mm, 20 * mm], font_size=7.2), Spacer(1, 5 * mm)]
for text in [
    "Use non-return valves so untreated and backwash water cannot enter the safe-water line.",
    "Backwash one media cartridge at a time in the field design. The simple prototype manifold can demonstrate the sequence, but it does not prove full-bed cleaning.",
    "Do not backwash a fluoride/adsorption cartridge unless the manufacturer or media study supports it; backwash can redistribute exhausted media but cannot regenerate sorption capacity.",
    "Require a backwash-tank high-level switch and source-tank low-level switch before enabling unattended automatic flushing.",
    "After backwash, divert initial rinse water to waste and require fresh safe sensor samples before enabling the tap.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [p("Trigger logic", styles["H2x"]), p("Establish a clean-bed flow baseline at the lowest intended tank level. Warn below 80% of baseline. During a genuine dispense request, if flow remains below 60% for 30 seconds, isolate, reverse-flush, settle and return to READY only after safe readings. Never interpret zero flow while nobody is requesting water as a clog.", styles["Bodyx"]), PageBreak()]

# Wiring
story += [p("7. Safe Arduino Mega electrical design", styles["H1x"])]
power_rows = [
    [p("Rail/device", styles["TableHead"]), p("Correct supply", styles["TableHead"]), p("Protection and wiring rule", styles["TableHead"])],
    [p("12 V valves/UV input", styles["TableText"]), p("Fused 12 V battery/adapter bus sized for simultaneous loads.", styles["TableText"]), p("Separate branch fuses; flyback diode on DC coils; never carry valve current through a breadboard or Mega pin.", styles["TableText"])],
    [p("Mega/sensors/relay logic", styles["TableText"]), p("Regulated 5 V buck with current margin and low ripple.", styles["TableText"]), p("Star ground, local decoupling, waterproof glands. Verify relay input polarity at reset.", styles["TableText"])],
    [p("SIM800L", styles["TableText"]), p("Dedicated 4.0 V rail, capable of 2 A bursts.", styles["TableText"]), p("At least 100 uF low-ESR close to module (470-1000 uF is commonly prudent), short wide conductors, logic-level shifting to Mega UART.", styles["TableText"])],
    [p("Analog probes", styles["TableText"]), p("Clean regulated 5 V only if module permits.", styles["TableText"]), p("Shielded/split routing from relays and antenna. Consider galvanic isolation or timed power sequencing for pH/TDS interference.", styles["TableText"])],
    [p("UV-C", styles["TableText"]), p("Manufacturer-rated driver, switched by suitable MOSFET/relay.", styles["TableText"]), p("Opaque closed chamber, door/cover interlock, current/intensity feedback. Never expose eyes or skin.", styles["TableText"])],
]
story += [table(power_rows, [36 * mm, 58 * mm, 75 * mm], font_size=7.25), Spacer(1, 5 * mm)]
story += [p("Arduino Mega pin map", styles["H2x"])]
pin_rows = [
    [p("Pin", styles["TableHead"]), p("Function", styles["TableHead"]), p("Connection", styles["TableHead"])],
    [p("D2", styles["TableText"]), p("Flow pulse interrupt", styles["TableText"]), p("YF-S201 yellow or replacement sensor pulse", styles["TableText"])],
    [p("D3", styles["TableText"]), p("Demand/authorization", styles["TableText"]), p("Active-low pushbutton or isolated RFID authorization output", styles["TableText"])],
    [p("D4", styles["TableText"]), p("Manual backwash", styles["TableText"]), p("Active-low service button", styles["TableText"])],
    [p("D6", styles["TableText"]), p("UV enable", styles["TableText"]), p("Logic-level MOSFET/driver input; not the LED power path", styles["TableText"])],
    [p("D7", styles["TableText"]), p("Local alarm", styles["TableText"]), p("Buzzer driver", styles["TableText"])],
    [p("D8-D11", styles["TableText"]), p("Four valves", styles["TableText"]), p("Tap, feed, backwash, waste relay inputs", styles["TableText"])],
    [p("A0-A4", styles["TableText"]), p("Analog sensing", styles["TableText"]), p("pH, TDS, turbidity, supply voltage, optional fluoride amplifier", styles["TableText"])],
    [p("Serial1 D18/D19", styles["TableText"]), p("SIM800L UART", styles["TableText"]), p("Bidirectional logic-level shifting; D18 TX1, D19 RX1", styles["TableText"])],
]
story += [table(pin_rows, [30 * mm, 52 * mm, 87 * mm], font_size=7.2), Spacer(1, 4 * mm)]
story += [p("Valve warning", styles["H2x"]), p("Confirm the exact CR02/CR03 actuator wiring before connection. A spring-return or power-open two-wire valve can be controlled by one switched channel; a polarity-reversing or separate OPEN/CLOSE actuator needs an H-bridge or two interlocked relays. The firmware assumes each valve can be commanded open by energizing one channel and returns closed when de-energized.", styles["Bodyx"]), PageBreak()]

# Firmware
story += [p("8. Firmware architecture and fail-closed behavior", styles["H1x"])]
story += [p("The supplied sketch targets Arduino Mega 2560, uses no third-party libraries and is split into configuration, measurement, sensor, policy, actuator and GSM modules. It avoids dynamic String allocation and long blocking delays.", styles["Bodyx"])]
state_rows = [
    [p("State", styles["TableHead"]), p("Tap", styles["TableHead"]), p("Purpose", styles["TableHead"])],
    [p("STARTUP_LOCKED", styles["TableText"]), p("Closed", styles["TableText"]), p("Wait for calibrated valid sensors and six consecutive safe samples.", styles["TableText"])],
    [p("READY", styles["TableText"]), p("Closed", styles["TableText"]), p("Feed path is available; awaits an active-low authorized demand.", styles["TableText"])],
    [p("UV_WARMUP", styles["TableText"]), p("Closed", styles["TableText"]), p("UV enabled for configured warm-up; cancellation returns to READY.", styles["TableText"])],
    [p("DISPENSING", styles["TableText"]), p("Open", styles["TableText"]), p("Only state that energizes the normally-closed drinking-water solenoid.", styles["TableText"])],
    [p("FAULT_LOCKED", styles["TableText"]), p("Closed", styles["TableText"]), p("Any invalid critical sensor or persistent unsafe result; local alarm and rate-limited GSM event.", styles["TableText"])],
    [p("BACKWASH_*", styles["TableText"]), p("Closed", styles["TableText"]), p("Isolate, reverse flush and settle; dispensing never resumes automatically.", styles["TableText"])],
]
story += [table(state_rows, [38 * mm, 28 * mm, 103 * mm], font_size=7.3), Spacer(1, 5 * mm)]
for text in [
    "All calibration flags ship false. This is intentional: the prototype cannot dispense on placeholder coefficients.",
    "Three consecutive ordinary unsafe samples lock the unit; extreme pH/turbidity/fluoride locks immediately. Six safe samples plus released demand are required to recover.",
    "The watchdog resets the controller after a software stall. Relay outputs are driven to the de-energized safe state before serial/sensor startup.",
    "SIM800L SMS/call handling is a non-blocking state machine and is disabled until a real phone number, power rail and bench test are supplied.",
    "CSV telemetry exposes state, measurements, warnings and fault bitmasks for judging and validation logs.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [p("Validation status", styles["H2x"]), p("The source has been compiled with strict host-side Arduino stubs and the safety-policy unit tests pass. That catches syntax and deterministic policy errors, but it is not a substitute for compiling with the installed Arduino AVR core, flashing a real Mega, injecting sensor voltages, checking relay reset states and testing power interruptions.", styles["Bodyx"])]
story += [p("Recommended next firmware increments", styles["H2x"])]
for text in [
    "Add source-low, backwash-ready and enclosure-open interlocks.",
    "Add UV current/intensity feedback; a commanded UV output is not proof of delivered dose.",
    "Store calibration coefficients and media service counters in EEPROM with version and CRC.",
    "Add an SD or FRAM event log so safety evidence does not depend on GSM coverage.",
    "Replace time-only backwash with per-cartridge differential pressure or validated flow recovery.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [PageBreak()]

# Calibration
story += [p("9. Calibration and validation protocol", styles["H1x"])]
cal_rows = [
    [p("Test", styles["TableHead"]), p("Minimum protocol", styles["TableHead"]), p("Acceptance decision", styles["TableHead"])],
    [p("pH", styles["TableText"]), p("Fresh pH 4.00, 7.00 and 10.00 buffers at known temperature; fit slope/intercept; independent check buffer before each trial day.", styles["TableText"]), p("Error <=0.1 pH across bracket; stable within declared response time.", styles["TableText"])],
    [p("Turbidity", styles["TableText"]), p("0, 1, 5 and 20 NTU standards in final opaque cell, including bubbles and ambient-light challenge.", styles["TableText"]), p("If uncertainty near 1 NTU cannot support the lock, label TS-300B trend-only and install a low-range meter.", styles["TableText"])],
    [p("TDS/conductivity", styles["TableText"]), p("Certified standards bracketing expected water; record temperature; compare against reference meter.", styles["TableText"]), p("Define matrix-specific error. Upgrade range before using any 2000 mg/L policy.", styles["TableText"])],
    [p("Flow", styles["TableText"]), p("Five gravimetric timed collections at high/mid/low tank levels, clean bed and partly loaded bed.", styles["TableText"]), p("Set pulses/L and clean-flow baseline from data; use a sensor whose specified range includes every test.", styles["TableText"])],
    [p("Fluoride", styles["TableText"]), p("0.5, 1.0, 1.5, 5, 10 and 15 mg/L standards/samples; TISAB-conditioned; standards bracket sample; lab cross-check.", styles["TableText"]), p("LOQ <=0.1 mg/L; bias small enough to defend 1.4 mg/L interlock guard band.", styles["TableText"])],
    [p("Media breakthrough", styles["TableText"]), p("Real source water, worst-case competing ions and target flow; sample every stage and cumulative bed volumes.", styles["TableText"]), p("Replacement point set before effluent reaches conservative warning; report capacity with uncertainty.", styles["TableText"])],
    [p("Microbiology/UV", styles["TableText"]), p("E. coli and total coliform, UV transmittance, validated dose at maximum flow, lamp aging and power variation.", styles["TableText"]), p("Not detectable in 100 mL; delivered UV dose meets validated design, commonly at least 40 mJ/cm2 for a conservative prototype target.", styles["TableText"])],
]
story += [table(cal_rows, [32 * mm, 85 * mm, 52 * mm], font_size=7.05), Spacer(1, 5 * mm)]
story += [p("Breakthrough study design", styles["H2x"])]
for text in [
    "Report influent chemistry, media mass/particle size, bed dimensions, flow, empty-bed contact time, pH, temperature and cumulative bed volumes.",
    "Plot effluent/influent concentration (C/C0) against bed volumes for fluoride and each target metal. Do not use a single removal percentage as cartridge life.",
    "Use actual Dhanbad water. Published fluoride values vary across sites and studies; design around verified source sampling rather than one headline maximum.",
    "Challenge at the lowest and highest operating pH and with relevant sulfate, bicarbonate, phosphate and competing ions.",
    "Retest after backwash and after storage to identify desorption, channeling and microbial regrowth.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [PageBreak()]

# FMEA
story += [p("10. Failure-mode and safety review", styles["H1x"])]
fmea_rows = [
    [p("Failure", styles["TableHead"]), p("Risk", styles["TableHead"]), p("Required response", styles["TableHead"]), p("Gap / addition", styles["TableHead"])],
    [p("Power loss/reset", styles["TableText"]), p("Untreated water dispensed", styles["TableText"]), p("Normally-closed tap and de-energized safe relay state.", styles["TableText"]), p("Bench-test reset transients and brownouts with every load switching.", styles["TableText"])],
    [p("Sensor unplugged/stuck", styles["TableText"]), p("False safe reading", styles["TableText"]), p("Range/plausibility failure locks tap; calibration flags default false.", styles["TableText"]), p("Add stuck-value/rate-of-change diagnostics and periodic reference checks.", styles["TableText"])],
    [p("High turbidity", styles["TableText"]), p("UV shielding", styles["TableText"]), p("Lock at 1 NTU project set-point; divert to waste.", styles["TableText"]), p("Use a validated low-range sensor and clean optical windows.", styles["TableText"])],
    [p("UV fails while commanded", styles["TableText"]), p("Microbial exposure", styles["TableText"]), p("Tap must close.", styles["TableText"]), p("Add intensity/current feedback; present hardware list lacks this proof.", styles["TableText"])],
    [p("Source empty", styles["TableText"]), p("False clog/backwash, pump/valve cycling", styles["TableText"]), p("Block dispense/backwash and alert.", styles["TableText"]), p("Add source low-level switch.", styles["TableText"])],
    [p("Backwash tank empty", styles["TableText"]), p("Dry backwash and no cleaning", styles["TableText"]), p("Abort and remain locked.", styles["TableText"]), p("Add backwash-ready level switch.", styles["TableText"])],
    [p("Valve stuck/miswired", styles["TableText"]), p("Cross-contamination", styles["TableText"]), p("Hydraulic check valves and position verification.", styles["TableText"]), p("Add limit feedback or flow-path proof for field unit.", styles["TableText"])],
    [p("GSM unavailable", styles["TableText"]), p("Remote alert missed", styles["TableText"]), p("Safety remains local and fail-closed; buzzer/log continues.", styles["TableText"]), p("Never make water safety dependent on cellular service.", styles["TableText"])],
    [p("Media exhausted", styles["TableText"]), p("Fluoride/metals breakthrough", styles["TableText"]), p("Replace on measured breakthrough/service counter; lock if direct fluoride rises.", styles["TableText"]), p("No sensor in current list proves metals removal; scheduled lab QA is mandatory.", styles["TableText"])],
]
story += [table(fmea_rows, [31 * mm, 38 * mm, 52 * mm, 48 * mm], font_size=6.85), Spacer(1, 6 * mm)]
safety_box = Table([[p("Drinking-water release rule", styles["CalloutTitle"]), p("Until the complete treatment train has passed chemical, metals, microbial and UV validation, label all output <b>NOT FOR DRINKING - TEST WATER ONLY</b>. Arduino interlocks reduce risk; they do not certify potability.", styles["Bodyx"])]], colWidths=[42 * mm, 127 * mm])
safety_box.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), PALE_RED), ("BOX", (0, 0), (-1, -1), 1.2, RED), ("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("PADDING", (0, 0), (-1, -1), 8)]))
story += [safety_box, PageBreak()]

# Demo and award narrative
story += [p("11. Demonstration and judging strategy", styles["H1x"])]
story += [p("The strongest presentation is not a claim that inexpensive sensors solve every contaminant. It is a transparent safety architecture that knows its measurement limits and produces auditable evidence.", styles["Bodyx"])]
demo_rows = [
    [p("Demo moment", styles["TableHead"]), p("What judges see", styles["TableHead"]), p("Evidence shown", styles["TableHead"])],
    [p("1. Gravity", styles["TableText"]), p("Water moves down the labelled staircase with no service pump.", styles["TableText"]), p("Measured raw-to-tap head and live flow at high/low tank levels.", styles["TableText"])],
    [p("2. Safe dispense", styles["TableText"]), p("Authorized demand starts UV warm-up, then opens NC tap.", styles["TableText"]), p("State and calibrated readings in serial dashboard/log.", styles["TableText"])],
    [p("3. Fault injection", styles["TableText"]), p("Unplug sensor or introduce a safe simulated over-limit voltage; tap closes and alarm/SMS event is queued.", styles["TableText"]), p("Fail-closed trace and fault bitmask; do not use hazardous contaminants on stage.", styles["TableText"])],
    [p("4. Clog/backwash", styles["TableText"]), p("Restricted line causes persistent low flow; service isolates and reverse route opens to waste.", styles["TableText"]), p("Before/after flow recovery; drinking path remains closed.", styles["TableText"])],
    [p("5. Science", styles["TableText"]), p("Team displays fluoride breakthrough and metals lab reports rather than relying on TDS.", styles["TableText"]), p("C/C0 curves, uncertainty, cartridge replacement point and source-specific treatment mode.", styles["TableText"])],
]
story += [table(demo_rows, [31 * mm, 72 * mm, 66 * mm], font_size=7.15), Spacer(1, 6 * mm)]
story += [p("Differentiators worth defending", styles["H2x"])]
for text in [
    "Source-adaptive mode selection: limestone is used only when source chemistry needs it.",
    "Direct fluoride side-stream with reagent isolation, rather than a false TDS proxy.",
    "Hardware fail-closed tap, verified sensor health and UV interlock path.",
    "Replaceable cartridges with measured breakthrough curves and local service counters.",
    "Gravity-first hydraulics and local safety behavior that continue without cloud or GSM.",
    "Evidence dashboard that reports uncertainty and explicitly marks parameters as online, inferred or laboratory-only.",
]:
    story.append(p("- " + text, styles["Bulletx"]))
story += [p("One-sentence pitch", styles["H2x"]), p("ProtoWave is a gravity-first, source-adaptive treatment platform whose Arduino Mega controller refuses to dispense when measured quality, sensor health or the disinfection barrier is unsafe - while direct fluoride and laboratory metals evidence prevent low-cost sensors from making claims they cannot support.", styles["Quote"]), PageBreak()]

# Sources
story += [p("12. Research basis and selected sources", styles["H1x"])]
sources = [
    ("Government of India, Jal Jeevan Mission - WQMS Framework / IS 10500 parameter table", "https://jaljeevanmission.gov.in/sites/default/files/manual_document/WQMS-Framework.pdf"),
    ("WHO - Fluoride in Drinking-water", "https://iris.who.int/bitstream/handle/10665/43514/9241563192_eng.pdf"),
    ("WHO - Water quality and health: review of turbidity", "https://www.who.int/publications/i/item/WHO-FWC-WSH-17.01"),
    ("Jal Jeevan Mission - Technical Expert Committee report on IoT water monitoring", "https://jaljeevanmission.gov.in/sites/default/files/manual_document/technical-expert-committee-report-on-measuring-and-monitoring_0.pdf"),
    ("Arduino - Mega 2560 Rev3 official documentation", "https://docs.arduino.cc/hardware/mega-2560/"),
    ("DFRobot - Analog TDS Sensor SEN0244 specifications and conversion", "https://wiki.dfrobot.com/sen0244"),
    ("DFRobot - Analog pH Sensor SEN0161 specifications", "https://wiki.dfrobot.com/sen0161"),
    ("DFRobot - Turbidity Sensor SEN0189 specifications", "https://wiki.dfrobot.com/sen0189"),
    ("Sensors 2020 - Low-cost turbidity sensor evaluation including TS-300B", "https://www.mdpi.com/1424-8220/20/7/1993"),
    ("DATAQ - YF-S201 flow sensor specifications", "https://www.dataq.com/products/accessories/flow-sensor/2000362.html"),
    ("SIMCom - SIM800L hardware design guide", "https://simcom.ee/documents/SIM800L/SIM800L(MT6261)_Hardware%20Design_V1.01.pdf"),
    ("NEMI - Standard Methods 4500-F C fluoride ion-selective electrode method", "https://www.nemi.gov/methods/method_summary/5709/"),
    ("Shams et al. - Calcium-modified biochar for fluoride removal", "https://pmc.ncbi.nlm.nih.gov/articles/PMC7970507/"),
    ("US EPA - Activated alumina fluoride treatment design manual", "https://nepis.epa.gov/Exe/ZyPURL.cgi?Dockey=P100KFZQ.TXT"),
    ("Motsi et al. - Natural zeolite removal of metals from acid mine drainage", "https://pubs.acs.org/doi/abs/10.1021/es048482s"),
    ("USGS - Limestone armoring in acid mine drainage systems", "https://pubs.usgs.gov/publication/70024876"),
    ("US EPA - Ultraviolet Disinfection Guidance Manual", "https://nepis.epa.gov/Exe/ZyPURL.cgi?Dockey=901T0000.TXT"),
    ("Portable flow anodic-stripping voltammetry for As, Pb and Cd", "https://pmc.ncbi.nlm.nih.gov/articles/PMC8892198/"),
    ("Qualitative assessment of mine water in Western Jharia Coalfield", "https://www.cwejournal.org/vol11no1/qualitative-assessment-of-mine-water-of-western-jharia-coal-field-jharkhandindia"),
    ("Groundwater fluoride dynamics and cited Dhanbad/Gharbar concentration context", "https://pubs.acs.org/aewcaa/article/6/6/3804/5185898/Groundwater-Fluoride-Dynamics-and-Potential-Risk"),
]
for i, (label, url) in enumerate(sources, start=1):
    story.append(p(f"{i}. {link(label, url)}", styles["Bodyx"]))
story += [Spacer(1, 4 * mm)]
story += [p("Interpretation note", styles["H2x"]), p("Literature removal efficiencies are not portable design capacities. Published Dhanbad-area source chemistry varies by site and season. The final media selection, service interval and release decision must be based on representative local samples, accredited analytical methods and a documented uncertainty budget.", styles["Bodyx"])]
story += [Spacer(1, 8 * mm)]
closing = Table([[p("DELIVERABLE STATUS", styles["CalloutTitle"]), p("Exact stepped placement schematic, labelled concept render, Arduino Mega firmware, host smoke test, safety-policy unit tests and this technical design are included. Remaining work is physical calibration, Arduino AVR compilation/flash testing, hydraulic commissioning and accredited water-quality validation.", styles["Bodyx"])]], colWidths=[42 * mm, 127 * mm])
closing.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), PALE_GREEN), ("BOX", (0, 0), (-1, -1), 1, TEAL), ("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("PADDING", (0, 0), (-1, -1), 8)]))
story += [closing]

OUT.parent.mkdir(parents=True, exist_ok=True)
doc.build(story)
print(OUT)

