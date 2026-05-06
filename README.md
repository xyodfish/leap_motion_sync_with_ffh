<p align="center">
  <strong>Leap Motion Sync With FFH</strong>
</p>

> 基于 Leap Motion v2 的灵巧手（FFH）实时同步与控制示例工程

<p align="center">
  <a href="#-项目简介">项目简介</a> •
  <a href="#-快速开始">快速开始</a> •
  <a href="#-运行示例">运行示例</a> •
  <a href="#-参数说明">参数说明</a> •
  <a href="#-故障排除">故障排除</a>
</p>

---

## ✨ 项目简介

这个仓库用于演示把 **Leap Motion** 手部跟踪数据实时映射到 **FFH 灵巧手** 控制命令。

核心能力：

- 通过 Leap Motion 捕捉手部与手指三维姿态
- 通过 UDP 向灵巧手发送关节控制指令
- 提供三种任务模式：实时同步、自检、按参数测试

---

## 🧩 系统组成

```text
Leap Motion 设备
  -> Leap SDK / Wrapper
  -> 同步与映射逻辑（task）
  -> UDP Client
  -> FFH 控制器（impedance_controller）
  -> 灵巧手执行
```

目录重点：

- `src/task/`：Leap 数据处理与任务逻辑
- `src/udp/`：UDP 通信
- `src/FFH/`：灵巧手相关数据结构与控制接口
- `config/leap_motion_demo_config.yaml`：主要运行参数
- `bin/`：示例可执行程序与动态库

---

## 📷 硬件示意

| Leap Motion 设备 | 灵巧手连接示意 |
| --- | --- |
| ![leap motion device](./doc/lp_device.png) | ![ffh hand device](./doc/handDevice.png) |

---

## 🧰 环境依赖

- Linux（Ubuntu）
- CMake
- C++ 编译器（支持 C++11/14/17，按当前工程配置）
- Leap Motion v2 驱动（`ultraleap-hand-tracking`）

驱动检测命令：

```bash
ultraleap-hand-tracking-control-panel
```

---

## 🚀 快速开始

### 1) 编译工程

```bash
cmake -S . -B build
cmake --build build -j"$(nproc)"
```

### 2) 连接硬件

- Leap Motion：USB Type-C 连接，侧灯绿色常亮
- FFH 灵巧手：按接线图连接，网线直连到 PC 网口

### 3) 启动灵巧手控制器

先查网口名：

```bash
ifconfig
```

启动控制器（将 `enp0s31f6` 替换为你的网口名）：

```bash
sudo ./impedance_controller enp0s31f6
```

---

## 🎮 运行示例

### 1) 角度实时同步（`leap_motion_demo`）

在 `config/leap_motion_demo_config.yaml` 设置：

```yaml
task_name: "leap_motion_demo"
```

运行：

```bash
./test_lpv2_demo
```

### 2) 手指自检（`self_check`）

配置：

```yaml
task_name: "self_check"
```

运行后按提示输入 `test` 开始，输入 `quit` 退出。

### 3) 手动参数测试（`test_by_hand`）

配置：

```yaml
task_name: "test_by_hand"
```

运行后输入 `test`，按提示输入手指 ID、关节 ID、目标角度。

---

## ⚙️ 参数说明

主配置文件：`config/leap_motion_demo_config.yaml`

常用参数：

- `connection_internal`：连接建立延时（s）
- `control_internal`：控制周期（s）
- `hand_Valid`：使能的灵巧手列表
- `task_name`：任务模式（`leap_motion_demo` / `self_check` / `test_by_hand`）
- `self_check_angle1` / `self_check_angle2`：自检角度模板
- `hand_info`：手部连接与关节限幅配置

---

## 🛠️ 故障排除

常见问题：手指无响应、运动异常。

建议处理顺序：

1. 断电重启灵巧手
2. 重新启动 `impedance_controller`
3. 确认 Leap 控制面板能识别手部
4. 检查配置文件中的网口与限幅参数

---

## 📎 参考资料

- 设备连线与界面截图：`doc/`
- 示例配置：`config/leap_motion_demo_config.yaml`
