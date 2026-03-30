# TCA6424 键盘驱动与功能验证方案

## 摘要
基于 `ESP32-S3-DevKitC` 新增一套 `TCA6424 + 键盘矩阵` 的可复用驱动和一个独立验证示例，用来确认键盘硬件、I2C 通讯、矩阵扫描和去抖逻辑是否正常。

本次已锁定的默认条件：
- I2C 使用 `SDA=GPIO16`、`SCL=GPIO15`
- TCA6424 I2C 地址固定为 `0x22`
- 验证目标为“通用功能验证”，输出矩阵键位编号/连线信息，不做业务键名映射
- v1 使用轮询扫描，不依赖 `INT` 中断脚

## 关键实现
### 1. 新增低层 TCA6424 驱动
在项目私有库里新增一个最小但完整的 `Tca6424` 驱动层，负责：
- `begin(TwoWire&, uint8_t address)`：绑定 I2C 总线并探测设备是否应答
- 24 位批量寄存器访问：一次性读 `Input Port 0..2`，写 `Output Port 0..2`，写 `Configuration Port 0..2`
- 维护 24 位 shadow 状态，避免每次扫描时拼散寄存器
- 所有寄存器访问统一使用 TCA6424 的 auto-increment 命令字
- I2C 读写失败时返回错误，不在驱动层打印串口

驱动层公开接口只保留：
- `bool probe()`
- `bool readInputs(uint32_t &value24)`
- `bool writeOutputs(uint32_t value24)`
- `bool writeConfig(uint32_t value24)`

### 2. 新增键盘扫描层
在低层驱动之上增加一个 `Tca6424KeyboardScanner`，实现“矩阵扫描 + 去抖 + 事件输出”。

实现方式固定为：
- 先根据 [T-KeyDriver.pdf](d:/dgx/code/T_Encoder_Plus_S3/hardware/T-KeyDriver.pdf) 提取实际 `row` / `col` 对应的 TCA6424 端口，并编码成两个 `constexpr` 数组
- 扫描时只驱动一根 `row` 为低电平，其余 `row` 输出高电平，所有 `col` 配成输入
- 每轮扫描生成一个矩阵位图，不直接输出瞬时变化
- 去抖采用 `10 ms` 扫描周期 + `3` 次一致判稳，即约 `30 ms`
- 事件只分 `PRESS` / `RELEASE`，不做长按、连发、组合键语义
- 支持多键同时按下
- 内部保存上一帧稳定状态，只在状态变化时生成事件

高层对外固定输出：
- 键位编号：`R{row}C{col}`
- 端口映射：`[row=Pxy col=Pab]`
- 事件类型：`PRESS` / `RELEASE`

### 3. 新增验证示例与构建入口
新增一个独立 example，不改现有 `IIC_Scan_2` 逻辑。

示例行为固定为：
- `Wire.begin(16, 15, 400000)`
- 上电先打印启动信息、I2C 引脚、地址、矩阵尺寸
- 先执行 `probe()`；如果 `0x22` 无响应，串口报错并每 1 秒重试
- 设备存在后开始持续扫描
- 每次键状态变化打印一行，例如：
  - `PRESS   R1C3 [row=P01 col=P14]`
  - `RELEASE R1C3 [row=P01 col=P14]`
- 保持串口输出只用于验证，不引入 UI、HID、LVGL 等额外依赖

`platformio.ini` 的变更固定为：
- 新增独立环境，例如 `Keyboard_TCA6424_Test`
- 保留当前 `default_envs` 不变，避免影响现有 I2C 示例
- 新环境目录名与 env 名保持一致，符合当前 `src_dir = examples/${platformio.default_envs}` 结构

## 公共接口与配置
新增的公共接口只包含两层：
- 低层 `Tca6424`
- 高层 `Tca6424KeyboardScanner`

新增的编译期配置固定放在扫描示例顶部或扫描器头文件中：
- `constexpr uint8_t kI2cAddress = 0x22;`
- `constexpr uint8_t kSdaPin = 16;`
- `constexpr uint8_t kSclPin = 15;`
- `constexpr uint32_t kI2cFreq = 400000;`
- `constexpr uint8_t kDebounceSamples = 3;`
- `constexpr uint16_t kScanPeriodMs = 10;`
- `constexpr std::array<uint8_t, N> kRowPins`
- `constexpr std::array<uint8_t, M> kColPins`

不引入运行时菜单、不做动态矩阵自发现、不增加额外配置文件。

## 测试方案
必须覆盖以下场景：
- 新 env 能成功编译
- 上电后能正确识别 `0x22`
- 未按键时串口不持续刷假事件
- 任意单键按下时只产生一次 `PRESS`
- 持续按住时不重复刷 `PRESS`
- 松开时只产生一次 `RELEASE`
- 同时按两个键时能各自独立上报
- TCA6424 断开或地址错误时不会死机，会持续报未连接并重试
- 去抖后不会因机械抖动产生连续误报

人工验收标准固定为：
- 所有物理键都至少能得到一组稳定的 `PRESS/RELEASE`
- 输出的 `R/C` 编号与从原理图得到的矩阵位置一一对应
- 连续按压 20 次以上无明显漏报或重复报

## 假设与默认
- 采用你确认后的 I2C 引脚：`GPIO16 / GPIO15`
- 本次只做通用验证，不输出最终业务键名；如果后续需要具体键帽名称，再单独加一张 `R/C -> KeyName` 映射表
- v1 不使用 `INT`，即使原理图上接出了中断脚，也先用轮询降低 bring-up 风险
- 扫描行列归属以 [T-KeyDriver.pdf](d:/dgx/code/T_Encoder_Plus_S3/hardware/T-KeyDriver.pdf) 为准，不凭猜测硬编码
- TCA6424 寄存器访问按 TI 官方寄存器定义实现：`Input 0x80..82`、`Output 0x84..86`、`Config 0x8C..8E`（AI 模式）

参考：
- 本地原理图资料：[T-KeyDriver.pdf](d:/dgx/code/T_Encoder_Plus_S3/hardware/T-KeyDriver.pdf)
- TI 官方数据手册：https://www.ti.com/lit/gpn/TCA6424
