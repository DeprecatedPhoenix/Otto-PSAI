# PSAI v2 — Pseudo-Sentient Artificial Intelligence

**By Depphen's Depot**

PSAI is a behavior system for the [Otto DIY](https://www.ottodiy.com/) robot that makes it feel as alive as possible on an Arduino Nano. No camera, no WiFi, no machine learning — just a HC-SR04 ultrasonic sensor, a buzzer, and a surprisingly deep personality architecture.

---

## What is PSAI?

Most Otto sketches tell the robot what to do. PSAI gives Otto a *mood* and lets the mood decide.

Otto runs a persistent emotional state that evolves over time based on interactions, isolation, and a little bit of randomness baked in to keep him unpredictable. The result is a robot that feels like it has a personality rather than a script.

---

## Mood System

Otto's mood is a single integer (`moodValue`, 0–100) that drifts continuously. It maps to five named emotional states:

| Range | Mood | Behavior |
|-------|------|----------|
| 0–20 | 😔 Melancholy | Slow, droopy, shies away from visitors |
| 21–40 | 😰 Anxious | Jittery, startles easily, erratic |
| 41–60 | 😐 Neutral | Curious, balanced, investigates things |
| 61–80 | 😄 Happy | Energetic, approaches visitors, dances |
| 81–100 | 🤩 Euphoric | Maximum chaos, full dance parties |

### Mood Drift
- Prolonged isolation nudges mood toward anxious/melancholy
- Positive interactions (someone approaches while Otto is happy) push mood up
- A random "intrusive thought" mechanic adds small unpredictable nudges so Otto never fully stabilizes
- Anxious and melancholy Otto can slowly self-recover over time (resilience)

---

## Proximity Reactions

Otto reacts differently to the same stimulus depending on his current mood:

- **Melancholy** — backs away slowly, plays a sad tone
- **Anxious** — hard startle, spins, freezes
- **Neutral** — curious lean-in, steps toward visitor
- **Happy** — excited approach, celebrates
- **Euphoric** — runs toward it, full dance party

---

## Boredom Escalation

The longer Otto goes without interaction, the more dramatic his idle behavior becomes:

| Time Alone | Boredom Level | Behavior |
|------------|---------------|----------|
| < 15s | 0 — Calm | Subtle sways, small wiggles |
| 15–45s | 1 — Restless | Turns, explores, emotional sounds |
| 45–90s | 2 — Bored | More frequent, mood-tinted actions |
| 90s+ | 3 — Dramatic | Full attention-seeking performances |

---

## Buzzer Personality

Each mood has its own melodic phrase — not just beeps:

- **Melancholy** — slow descending minor phrase
- **Anxious** — rapid staccato, slightly dissonant
- **Neutral** — simple curious two-note question
- **Happy** — ascending major arpeggio
- **Euphoric** — fast chaotic celebratory burst

---

## Hardware Requirements

- Otto DIY robot (standard build)
- Arduino Nano (ATmega328P)
- HC-SR04 ultrasonic sensor
- Passive buzzer
- OttoDIYLib

### Pin Defaults

| Component | Pins |
|-----------|------|
| Left Leg | 2 |
| Right Leg | 3 |
| Left Foot | 4 |
| Right Foot | 5 |
| Buzzer | 13 |
| HC-SR04 Trig | 9 |
| HC-SR04 Echo | 8 |

---

## Installation

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install **OttoDIY** via Sketch → Include Library → Manage Libraries
3. Open `Otto_PSAI_v2.ino`
4. Set board to **Arduino Nano**, processor to **ATmega328P (Old Bootloader)** if using a clone
5. Select your COM port and upload

> **Tip:** If uploading fails, make sure Otto's power switch is ON while the USB is connected. This lets the servos draw from the battery instead of the USB line, preventing brown-out during upload.

---

## Changelog

### v2 (current)
- Full persistent mood system (0–100 scale, 5 named states)
- Mood-dependent proximity reactions
- Boredom escalation with 4 drama levels
- Melodic buzzer phrases per mood
- Intrusive thought mechanic for unpredictability
- Millis()-based non-blocking main loop
- Fixed all bugs from v1

### v1
- Initial concept — random action selection via integer
- Basic proximity reaction
- Laid the groundwork for PSAI

---

## License

MIT License — free to use, modify, and build on. Credit appreciated but not required.

---

*Depphen's Depot — building things that probably shouldn't work but somehow do.*
