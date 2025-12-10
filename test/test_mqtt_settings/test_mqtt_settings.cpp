#include <Arduino.h>
#include <unity.h>

#include "Settings/Models/MqttSettings.h"

void test_command_topic_placeholder_is_replaced()
{
    MqttSettings settings;
    settings.command_topic = "device/{device_id}/command";

    String topic = settings.getCommandTopicForDevice("abc123");

    TEST_ASSERT_EQUAL_STRING("device/abc123/command", topic.c_str());
}

void test_command_topic_without_placeholder_is_preserved()
{
    MqttSettings settings;
    settings.command_topic = "device/+/command";

    String topic = settings.getCommandTopicForDevice("ignored");

    TEST_ASSERT_EQUAL_STRING("device/+/command", topic.c_str());
}

void test_has_secondary_requires_host_and_port()
{
    MqttSettings settings;
    settings.secondary.host = "";
    settings.secondary.port = 8883;
    TEST_ASSERT_FALSE(settings.hasSecondary());

    settings.secondary.host = "broker.example.com";
    settings.secondary.port = 0;
    TEST_ASSERT_FALSE(settings.hasSecondary());

    settings.secondary.host = "broker.example.com";
    settings.secondary.port = 8883;
    TEST_ASSERT_TRUE(settings.hasSecondary());
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_command_topic_placeholder_is_replaced);
    RUN_TEST(test_command_topic_without_placeholder_is_preserved);
    RUN_TEST(test_has_secondary_requires_host_and_port);
    UNITY_END();
}

void loop()
{
    // not used
}
