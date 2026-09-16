from __future__ import annotations
import json
from pathlib import Path
from internet_host import InternetHostPolicy, ingest, make_record
from knowledge_gateway import ResearchQuestion


def run() -> dict[str, object]:
    question = ResearchQuestion("physical-host-prep", "ESP32-S3 and T3-S3 capabilities", frozenset({"esp32", "psram", "radio", "usb", "wifi", "bluetooth"}))
    policy = InternetHostPolicy(frozenset({"wiki.lilygo.cc", "documentation.espressif.com"}))
    records = (
        make_record(
            "https://wiki.lilygo.cc/products/t3-series/t3-s3-v1.3/",
            "LILYGO T3-S3 V1.3 documentation",
            "T3-S3 V1.3 uses ESP32-S3FH4R2 dual-core LX7 at 240 MHz, includes 4 MB Flash, 2 MB QSPI PSRAM, USB-C, OLED, TF card, and optional SX1262/SX1276/SX1278/SX1280 LoRa modules. The SX1262 mapping lists LoRa SCK GPIO5, MISO GPIO3, MOSI GPIO6, RESET GPIO8, DIO1 GPIO33, BUSY GPIO34, CS GPIO7.",
            ("esp32", "psram", "usb", "radio"),
        ),
        make_record(
            "https://documentation.espressif.com/esp32-s3_datasheet_en.pdf",
            "Espressif ESP32-S3 datasheet",
            "ESP32-S3 is a dual-core Xtensa LX7 SoC up to 240 MHz with Wi-Fi 2.4 GHz, Bluetooth LE, USB Serial/JTAG, GPIO, UART, I2C, SPI, ADC and low-power modes.",
            ("esp32", "wifi", "bluetooth", "usb"),
        ),
    )
    result = ingest(records, question, policy)
    output = {
        "schema": "herus.internet-host-evidence.v1",
        "source_count": len(records),
        "accepted_count": len(result.accepted),
        "useful_claims": list(result.useful_claims),
        "decision_reasons": [decision.reason for decision in result.decisions],
        "executable": result.executable,
        "training_action": "not_applied_to_authority_or_firmware",
        "interpretation": "Internet supplied bounded documentation evidence useful for later H1/H2 planning; it did not modify executable code, grant authority, or replace physical measurement.",
    }
    Path(__file__).parent.joinpath("evidence", "internet_host_evidence.json").write_text(json.dumps(output, indent=2, ensure_ascii=False) + "\n")
    return output

if __name__ == "__main__":
    print(json.dumps(run(), indent=2, ensure_ascii=False))
