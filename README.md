# Switch 433


这是一个利用 NodeMCU 接入 433Mhz 射频到 HomeAssistant 的项目。

## RoadMap
> WIP: 等待射频模块到货，现在你只能开关板载的 LED 灯

- [x] WiFi / MQTT 对于 HA 的接入
- [ ] 射频学习
- [ ] 射频开关

## 缘由

租房处开关较难拆卸且为明装，关灯神器只有贴开关上的那种才能正常使用，而此种开关目前只看到了射频的版本，所以需要一个能够接入 HomeAssistant 的 433Mhz 射频网关。

相较于花费 70r+ 的 Sonoff Bridge 与 100r+ 的小米音箱射频加装，显然还是包邮的 `10r MCU` + `6r 射频模块` 更有性价比。

## 安装方式

1. 编辑 src/main.cpp，填入 WiFi 以及 MQTT 信息。
2. 在 HA 的 `configuration.yaml` 中添加以下配置：

```yaml
mqtt:
  - switch:
      unique_id: switch433
      name: "Switch 433"
      state_topic: "home/switch433"
      command_topic: "home/switch433/set"
      payload_on: "ON"
      payload_off: "OFF"
      state_on: "ON"
      state_off: "OFF"
      optimistic: false
      qos: 0
      retain: true
```

## 实现参考
- https://nodemcu.readthedocs.io/en/release/
- https://www.jianshu.com/p/1602fa02185e
