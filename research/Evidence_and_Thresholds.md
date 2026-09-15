# Evidence and operating thresholds

## Three different numbers must not be confused

1. A **drinking-water limit** is the concentration or range in a standard.
2. An **instrument detection/quantification level** is the smallest result the
   method can measure reliably.
3. A **controller set-point** is an engineering decision that includes a guard
   band, sensor uncertainty, and the consequence of failure.

The proposed controller set-points below are project decisions. They are not a
replacement for BIS/ISO laboratory testing.

## Online control matrix

| Parameter | Indian reference value | Project warning | Project lock | Required online capability | Status of listed sensor |
|---|---:|---:|---:|---|---|
| pH | 6.5-8.5, no relaxation | <6.7 or >8.3 | <6.5 or >8.5; immediate below 5 or above 10 | Verified error <=0.1 pH after 3-point calibration | PH-4502C can support a prototype only after calibration |
| Turbidity | 1 NTU acceptable; 5 NTU permissible | >=0.8 NTU | >=1.0 NTU because UV follows | 0-10 NTU range, resolution <=0.1 NTU, uncertainty small enough at 1 NTU | TS-300B is a trend sensor; upgrade for compliance |
| TDS | 500 mg/L acceptable; 2000 mg/L permissible without an alternate source | >500 mg/L | >=900 mg/L with the listed 0-1000 ppm board because the true value is becoming unknown | Validated range beyond the chosen regulatory limit | The listed board cannot verify 2000 mg/L and is non-specific |
| Fluoride | 1.0 mg/L acceptable; 1.5 mg/L permissible | >=1.0 mg/L | >=1.4 mg/L; immediate >=2.0 mg/L | LOQ <=0.1 mg/L across at least 0.1-20 mg/L | Not measured by the listed sensors; add a conditioned ISE/colorimetric method |
| Flow | Process-specific | <80% of clean-bed baseline | Backwash after <60% for 30 s during demand | Calibrated over actual gravity range | YF-S201 is normally unsuitable below about 1 L/min |
| 12 V supply | Engineering parameter | <11.2 V | <10.8 V | Calibrated divider; independent brownout protection | Listed voltage module is adequate after calibration |

TDS is deliberately a warning at 500 mg/L rather than a toxicity claim. In a
public dispenser, policy must explicitly choose the 500 mg/L acceptable limit
or the conditional 2000 mg/L limit. The supplied 0-1000 ppm module is not fit to
make that second decision.

## Metals: laboratory release criteria

| Analyte | Acceptable/permissible value (mg/L) | Project analytical LOQ goal (mg/L) | Online status |
|---|---:|---:|---|
| Arsenic | 0.01 / 0.05 | <=0.001 | Not measured |
| Lead | 0.01 / no relaxation | <=0.001 | Not measured |
| Cadmium | 0.003 / no relaxation | <=0.0003 | Not measured |
| Total chromium | 0.05 / no relaxation | <=0.005 | Not measured |
| Nickel | 0.02 / no relaxation | <=0.002 | Not measured |
| Mercury | 0.001 / no relaxation | <=0.0001 | Not measured |
| Manganese | 0.1 / 0.3 | <=0.01 | Not measured |
| Copper | 0.05 / 1.5 | <=0.005 | Not measured |
| Iron | 1.0 / no relaxation | <=0.1 | Not measured |

The LOQ goals are project quality targets of roughly one-tenth of the applicable
limit, not statutory values. Use an accredited laboratory with ICP-MS/AAS or an
equivalent validated method. Anodic-stripping voltammetry is promising for an
advanced prototype but must be validated against the actual Dhanbad water matrix.

## Source-water and media conclusions

- Published Dhanbad-area reports show severe fluoride contamination in some
  groundwater, including a cited maximum near 16.2 mg/L. A prototype should be
  challenged across 0.5, 1.0, 1.5, 5, 10, and 15 mg/L fluoride rather than only
  clean laboratory water.
- Other Jharia mine-water observations reported pH around 6.8-8.3. Therefore
  limestone must be bypassable; continuous limestone contact can raise pH and
  interfere with fluoride sorption when influent water is already neutral.
- Activated alumina is an established fluoride-treatment medium, but capacity
  depends strongly on pH, competing ions, and empty-bed contact time.
- Natural zeolite and modified biochar can remove some metals, but the performance
  of local media must be proven with breakthrough curves, not assumed from a
  batch-study removal percentage.
- Limestone can neutralize acidic drainage but may armor or clog as precipitates
  form. A settling/oxidation step and serviceable cartridges are preferable to
  one permanently mixed media bed.

## Validation protocol

1. Run a raw-water characterization panel before selecting treatment mode.
2. Calibrate pH at 4.00, 7.00 and 10.00; verify an independent check buffer.
3. Calibrate turbidity at 0, 1, 5 and 20 NTU; reject the sensor if uncertainty
   around 1 NTU is too large.
4. Calibrate TDS/conductivity with certified standards at measured temperature.
5. Calibrate flow gravimetrically using five repeated collections at multiple
   tank levels and with both clean and partly loaded media.
6. For a fluoride ISE, condition every standard/sample with TISAB, use standards
   that bracket the sample, and discharge the reagent stream to waste.
7. Run breakthrough tests using real source water, including worst-case fluoride
   and metals, and define cartridge replacement from the first conservative
   warning threshold.
8. Validate the complete unit for E. coli/total coliform (not detectable in
   100 mL) and UV dose. Do not release drinking water merely because chemistry
   readings look normal.

## Selected sources

- Government of India, Jal Jeevan Mission, *Uniform Drinking Water Quality
  Monitoring Protocol / WQMS Framework*: https://jaljeevanmission.gov.in/sites/default/files/manual_document/WQMS-Framework.pdf
- WHO, *Fluoride in Drinking-water*: https://iris.who.int/bitstream/handle/10665/43514/9241563192_eng.pdf
- WHO, *Water quality and health - review of turbidity*: https://www.who.int/publications/i/item/WHO-FWC-WSH-17.01
- DFRobot, Analog TDS Sensor SEN0244: https://wiki.dfrobot.com/sen0244
- DFRobot, Analog pH Sensor SEN0161: https://wiki.dfrobot.com/sen0161
- DFRobot, Turbidity Sensor SEN0189: https://wiki.dfrobot.com/sen0189
- DATAQ, YF-S201 flow sensor specifications: https://www.dataq.com/products/accessories/flow-sensor/2000362.html
- SIMCom, SIM800L hardware design guide: https://simcom.ee/documents/SIM800L/SIM800L(MT6261)_Hardware%20Design_V1.01.pdf
- NEMI, Standard Methods 4500-F C fluoride ISE method summary: https://www.nemi.gov/methods/method_summary/5709/
- Shams et al., calcium-modified biochar for fluoride: https://pmc.ncbi.nlm.nih.gov/articles/PMC7970507/
- EPA, activated alumina design manual: https://nepis.epa.gov/Exe/ZyPURL.cgi?Dockey=P100KFZQ.TXT
- Motsi et al., natural zeolite for mine-water metals: https://pubs.acs.org/doi/abs/10.1021/es048482s
- USGS, limestone armoring in acid mine drainage: https://pubs.usgs.gov/publication/70024876
- EPA, UV Disinfection Guidance Manual: https://nepis.epa.gov/Exe/ZyPURL.cgi?Dockey=901T0000.TXT
- MDPI Sensors, low-cost turbidity sensor evaluation: https://www.mdpi.com/1424-8220/20/7/1993
- Portable flow anodic-stripping voltammetry for As/Pb/Cd: https://pmc.ncbi.nlm.nih.gov/articles/PMC8892198/
- Jharia mine-water quality study: https://www.cwejournal.org/vol11no1/qualitative-assessment-of-mine-water-of-western-jharia-coal-field-jharkhandindia

