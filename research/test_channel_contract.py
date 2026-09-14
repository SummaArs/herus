import unittest

from channel_contract import Channel, ChannelRole, channel_for


class ChannelContractTests(unittest.TestCase):
    def test_ble_requires_authentication_even_for_allowed_effect(self):
        contract = channel_for(Channel.BLE)
        self.assertTrue(contract.can_transport("HAPTIC_FEEDBACK", True))
        self.assertFalse(contract.can_transport("HAPTIC_FEEDBACK", False))

    def test_observation_never_transports_control_effect(self):
        contract = channel_for(Channel.SPI)
        self.assertEqual(contract.role, ChannelRole.CONTROL)
        self.assertFalse(contract.can_transport("PROPOSE_ACTION", True))

    def test_i2c_is_limited_to_local_telemetry_and_haptics(self):
        contract = channel_for(Channel.I2C)
        self.assertFalse(contract.can_transport("HAPTIC_FEEDBACK", True))
        self.assertFalse(contract.can_transport("MOVE_ROBOT", True))

    def test_lora_payload_is_bounded(self):
        self.assertEqual(channel_for(Channel.LORA).max_payload, 222)


if __name__ == "__main__":
    unittest.main()
