# AI Vision Add-on Specification

## Overview

The AI Vision Add-on extends Pravi into Phase 4 by adding computer vision-based plant health analysis. Image capture happens at the edge (camera peripheral), inference runs in the Go-Gateway via a vision-capable AI model (e.g. Claude API), and results feed back into the existing watering and alert systems.

This add-on does not require firmware changes to the core state machine — it operates as a software layer above the serial telemetry bridge established in Phase 3.

---

## 1. Hardware: Camera Module

### MOD_ID Extension

The existing resistor-based module identification scheme (Pin 8 of the Pravi Port) is extended with a new entry:

| Resistor Value | Identified Module |
|:---|:---|
| 1.0 kΩ | Base Capacitive Moisture Sensor |
| 4.7 kΩ | Reservoir Kit (Pump + Level Sensor) |
| 10.0 kΩ | Direct-Line Kit (Solenoid + Flow Meter) |
| 22.0 kΩ | AI Vision Camera Module |

### Recommended Hardware

| Component | Role |
|:---|:---|
| Raspberry Pi Zero 2 W (or similar edge SBC) | Camera host, image capture, serial bridge to Go-Gateway |
| Raspberry Pi Camera Module 3 (or USB equivalent) | Primary image capture |
| Macro lens attachment (optional) | Root zone and close-up leaf inspection |
| Pan/tilt servo bracket (optional) | Multi-plant coverage from a single camera |

The camera module connects to the Pravi Port via the I2C lines (Pins 4/5) for control signaling and communicates image data to the Go-Gateway over the existing local network or USB serial link.

---

## 2. System Architecture

```
[Pravi Port — Camera Module]
        |
        | (I2C trigger / capture command)
        v
[Raspberry Pi / Edge SBC]
        |
        | (HTTP POST — base64-encoded JPEG frame)
        v
[Go-Gateway — Vision Service]
        |
        | (Anthropic API — multimodal request)
        v
[Claude Vision Model]
        |
        | (Structured JSON diagnosis)
        v
[Go-Gateway — Response Handler]
       / \
      /   \
     v     v
[Dashboard] [Automated Action Engine]
                    |
                    v
         [Watering / Alert / Log]
```

---

## 3. Go-Gateway: Vision Service

The vision service runs as a module within the Go-Gateway. It exposes an internal endpoint that the camera agent posts frames to, then forwards the request to the Claude API.

### Capture Trigger

Captures are scheduled on a configurable interval (default: every 6 hours) or triggered on-demand by sensor anomalies (e.g. moisture remains low after a watering cycle).

### Claude API Request Structure

```go
type VisionRequest struct {
    PlantID     string  `json:"plant_id"`
    ImageBase64 string  `json:"image_base64"`
    MoisturePct float64 `json:"moisture_pct"`
    TempC       float64 `json:"temp_c"`
    NPK         NPKReading `json:"npk"`
}
```

The Go service constructs a multimodal message to the Claude API combining:
- The captured image (base64-encoded)
- A structured system prompt defining the diagnostic schema
- Live sensor readings for context

### Example System Prompt

```
You are a plant health diagnostics assistant. Analyze the provided image and sensor data.
Return a JSON object with the following fields:
- "health_status": one of ["healthy", "warning", "critical"]
- "issues": array of detected problems (e.g. ["yellowing leaves", "aphid infestation", "wilting"])
- "confidence": float 0.0–1.0
- "recommended_action": short actionable string
- "sensor_notes": any observations about the provided sensor readings relative to the visual state
```

### Example Claude API Call (Go)

```go
import "github.com/anthropics/anthropic-sdk-go"

func analyzeFrame(req VisionRequest) (*DiagnosisResult, error) {
    client := anthropic.NewClient() // uses ANTHROPIC_API_KEY env var

    msg, err := client.Messages.New(context.TODO(), anthropic.MessageNewParams{
        Model:     anthropic.F(anthropic.ModelClaude_Opus_4_6),
        MaxTokens: anthropic.F(int64(1024)),
        Messages: anthropic.F([]anthropic.MessageParam{
            anthropic.NewUserMessage(
                anthropic.NewImageBlockBase64("image/jpeg", req.ImageBase64),
                anthropic.NewTextBlock(buildDiagnosticPrompt(req)),
            ),
        }),
    })
    if err != nil {
        return nil, err
    }

    return parseDiagnosis(msg.Content[0].Text), nil
}
```

---

## 4. Sensor Fusion

The real diagnostic advantage comes from correlating visual output with live sensor telemetry. The Go-Gateway merges both data streams before making the API call.

| Scenario | Visual Signal | Sensor Signal | Fused Diagnosis |
|:---|:---|:---|:---|
| Overwatering | Yellowing lower leaves, wilting | High moisture, low NPK | High confidence overwatering |
| Underwatering | Leaf curl, browning tips | Low moisture | High confidence drought stress |
| Nutrient deficiency | Interveinal chlorosis | Normal moisture, low NPK | Likely iron or magnesium deficiency |
| Pest damage | Stippling, webbing | Normal all sensors | Visual-only detection; flag for manual inspection |
| False alarm | Slightly drooped leaves | Adequate moisture, post-watering | Low severity; normal recovery state |

---

## 5. Add-on Variants

### Visual Health Monitor (Core)
Scheduled captures with AI diagnosis. Issues logged to dashboard, critical findings trigger SMTP alert (Phase 3 notification engine).

### Growth Tracker
Stores a daily reference frame per plant. Compares current frame to baseline using the AI model to detect stunted growth, unexpected changes, or progress milestones.

### Multi-Plant Triage
Pan/tilt bracket sweeps across multiple pots on a schedule. Each frame is tagged with a plant ID and analyzed independently. Results are ranked by severity so the system prioritizes the most distressed plant in the next watering cycle.

### Root Zone Camera (Advanced)
Macro lens positioned at soil level to visually confirm surface moisture and detect mold, fungus gnats, or root rot at the base of the stem — conditions that sensors alone may not catch.

---

## 6. Data Flow & Storage

All diagnosis results are stored in the Go-Gateway's local database (SQLite or Postgres) with the following schema:

```sql
CREATE TABLE diagnoses (
    id          INTEGER PRIMARY KEY,
    plant_id    TEXT NOT NULL,
    captured_at TIMESTAMP NOT NULL,
    health_status TEXT NOT NULL,         -- healthy / warning / critical
    issues      TEXT,                    -- JSON array
    confidence  REAL,
    action      TEXT,
    moisture_pct REAL,
    temp_c      REAL,
    image_path  TEXT                     -- local path to stored frame
);
```

Frames are retained for 30 days by default (configurable). The dashboard surfaces a timeline view per plant showing health history alongside sensor trends.

---

## 7. Privacy & Security

- All image processing happens locally or via direct API call — no third-party cloud storage.
- The `ANTHROPIC_API_KEY` is stored as an environment variable, never in source.
- Captured frames are stored on the local gateway machine only and purged on the configured retention schedule.
