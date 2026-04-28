# CLAUDE.md

> 给后续 Claude 会话用的项目上下文。中文为主，标识符/路径保留英文。

## 1. 项目概览

**GhostHunter** 是一个用 C++ / OpenGL 写的简易 FPS：玩家拿吸尘器在密室里抓 4 只鬼。撞到鬼则输，把鬼全部吸完则赢，开局有 5 秒冻结倒计时。

代码源自学生时期作业，**目前只能在 Visual Studio 2022 + Windows 上构建**。本次重构的总目标：把它清理到敢于 `Public` 的程度，并最终具备跨平台能力。

游戏玩法和操作映射本次不变，重构只动结构。

## 2. 技术栈与构建

| 类别 | 选型 |
| --- | --- |
| 语言 | C++（用到了 `<atomic>` / `<thread>` / `std::filesystem` 候选项，按 C++17 处理） |
| 渲染 | OpenGL 3.3 Core |
| 窗口/输入 | GLFW |
| GL 函数加载 | GLAD |
| 数学 | GLM |
| 模型导入 | Assimp |
| 纹理 | stb_image |
| 构建 | `GhostHunter.sln` + `.vcxproj`，PlatformToolset v143，Debug/Release × Win32/x64 |
| 属性表 | `config/OpenGL.props`、`config/Assimp.props` |

**构建/运行（当前）**：

- VS2022 打开 `GhostHunter.sln` 直接构建。
- 直接运行 `./Excutable/GhostHunter/GhostHunter.exe`（注意 cwd 必须能解析 `../res/...` 相对路径）。

**操作映射**：WASD 移动 / 鼠标视角 / 左键吸鬼 / `Space` 跳 / `Esc` 退出 / `P` 暂停。

## 3. 目录与文件地图

```
GhostHunter/
├── GhostHunter.sln                   # VS 解决方案
├── GhostHunter/                      # VS 工程目录（注意目录名和 sln 同名）
│   ├── GhostHunter.vcxproj
│   ├── src/{main.cpp, glad.c}        # 入口 + GLAD 实现
│   └── assimp-vc143-mt.dll           # 重复的二进制（坏味道）
├── common/                           # 真正的游戏逻辑
│   ├── World.h                       # 地图类（继承 Model），头文件里全是实现
│   ├── ghost.{h,cpp}                 # 鬼类，h 是 623 行巨型 header；cpp 是空文件
│   ├── player.{h,cpp}                # 玩家类（FPS 控制 + 碰撞 + HUD）
│   ├── camera.{h,cpp}                # 相机基类
│   ├── admin.{h,cpp}                 # 调试用飞行相机（main 没引用）
│   ├── shaders.{h,cpp}               # Shader 类，构造和 LoadShaders 重复
│   ├── tools.{h,cpp}                 # GLTools / Timer / MouseInfo / GameStatus / GLFW 回调
│   └── model/{mesh,model}.{h,cpp}    # Assimp 模型加载
├── config/{OpenGL,Assimp}.props      # VS 属性表
├── include/                          # vendored 头：GLFW/GLAD/GLM/assimp/stb_image
├── lib/                              # vendored 库：glfw3*.lib/dll、assimp lib
├── res/{model,shaders}/              # 资源：模型 obj + 着色器
├── Excutable/                        # 编译产物（拼写错，应为 Executable）
└── screenshoots/                     # README 用的截图（也拼错，应为 screenshots）
```

**关键入口**：

- `GhostHunter/src/main.cpp:28` — `main()`，包含游戏主循环、`ghostKillDetect`、`winDetect`。
- `common/ghost.h:15` — `class Ghost`，本项目的最大单文件复杂度集中地。

## 4. 现状（屎山点清单）

按严重程度分级。Claude 改代码前先看这一节，避免被既有代码误导。

### 4.1 严重（功能 / 稳定性问题）

- **多线程没同步**。`Ghost::runAwayTFun`、`captureTFunc`、`twinklingTFunc`、`looming`、`setGhostAlpha`、`setAmbient` 全是 `std::thread` + 立即 `detach()`。这些线程跨线程读写 `position` / `scale` / `health` / `drawEnable` / `ghostAlpha` 等成员，**没有任何锁**，部分用 `std::atomic<bool>` 但只是 flag，数据本身不是 atomic。OpenGL 上下文（吸尘动画里的 `glfwGetMouseButton`、`glfwGetTime`）也被工作线程触碰。`Ghost::animationThreads` push 之后立即 detach，vector 只增不减。
- **`Ghost::setAmbient` 是空转 NaN 循环**（`common/ghost.h:330`）。`std::abs(ambient - ambient) / (ambient - ambient)` 永远是 `NaN`，循环条件比较恒为 false 或恒为 true（取决于 NaN 语义），逻辑等同空转。属于"看起来在干活实际啥都没干还烧 CPU"。
- **`World::drawCollisionBoxes` GL buffer size 写错**（`common/World.h:87`）。`glBufferData(..., sizeof(vertices), &vertices, ...)` 传的是 `std::vector` 对象本身的 sizeof 和指向 vector 对象（不是数据）的指针，应是 `vertices.size() * sizeof(glm::vec3)` 和 `vertices.data()`。该函数注释自己也写了 "can't draw boxes"。
- **`World::setOffset(glm::vec3 scale)`**（`common/World.h:52`）。形参名叫 `scale`，函数体 `this->offset = offset` 是用成员自赋值，形参完全被忽略。
- **`Ghost` 析构对已 detach 的线程再 detach**（`common/ghost.h:94`）。`~Ghost` 里 `if (xxxThread.joinable()) xxxThread.detach()`，但这些线程在创建时基本已立即 detach，再 detach 是 UB；正确做法应该是改成可中断的协程式 update。
- **空 cpp 文件**：`common/ghost.cpp` 0 行；声明全在 header 里。

### 4.2 中等（结构 / 可维护性）

- **header-only 巨类**。`common/ghost.h` 623 行 / `common/player.h` 301 行 / `common/World.h` 202 行，所有方法 inline 在头里，编译慢，任何小改动都引发大量重编译。
- **全局变量满天飞**。`tools.h` 里 `extern GLFWwindow* window;` `extern MouseInfo mouseInfo;` `extern GLsizei window_width, window_height;`，多个 `.cpp` 里各自定义。GLFW 回调（`mouse_callback` / `framebuffer_size_callback`）全靠这些全局拿状态，无法多窗口。
- **`Shader` 构造函数与 `LoadShaders()` 完全重复**（`common/shaders.cpp`）。两份近 100 行近乎相同的 shader 编译/链接代码；编译失败时还 `getchar(); exit(-1)` 阻塞退出。
- **资源路径硬编码**。`Player` 的成员默认初始化里写死了 `Model("../res/model/vacuum/vacuum.obj")` 等等（`common/player.h:162`），路径相对 cwd 而非 exe 所在目录，换工作目录就崩。
- **`Model::draw` 按值拷贝 Mesh**（`common/model/model.cpp:7`）。`for (Mesh mesh : meshes)` 每帧把全部 mesh 拷一遍。
- **游戏状态用一堆散落的 bool**（`GhostHunter/src/main.cpp:94-99`）。`isLose / isWin / isFrozen / gameOver / closeWindow` 互相耦合，没状态机。
- **`Model::getTheCollideBox` 把 mesh 索引当地图魔数**（`common/model/model.cpp:170`）。`meshes[0~15]` 是柱子、`meshes[22]` 是栅栏、`meshes[33~35]` 是带洞墙、`meshes[36~46]` 是 6 个箱子……强耦合到具体 obj 文件结构，换地图就废。

### 4.3 轻微（卫生问题）

- **重复的二进制进 git**：`assimp-vc143-mt.dll` 同时存在于 `lib/`、`GhostHunter/`、`Excutable/GhostHunter/`；`stb_image.h`（28 万字符）、`glfw3*.lib/dll`、`assimp-vc143-mt.lib` 都在仓库里。
- **拼写错误**：目录 `Excutable/` 应为 `Executable/`；`screenshoots/` 应为 `screenshots/`。
- **乱码注释**：`World.h` 的 `drawCollisionBoxes`、`model/{model,mesh}.h` 部分注释为 GBK 写入的中文，被 UTF-8 工具读出全是 `���`。
- **`Admin` 类没人用**：`common/admin.{h,cpp}` 定义了飞行调试相机但 `main.cpp` 没引用。
- **缺 `.gitignore`**：仓库里没看到，构建产物可能被误提交。
- **缺开源元素**：仅有 `LICENSE`，没有 `.gitignore` / CI / `CONTRIBUTING.md` / issue 模板。

## 5. 重构方向（激进现代化路线）

按阶段推进，每阶段独立可发布、可单独 review。

### 阶段 1：构建系统现代化

- **CMake** 全面替换 `.sln` / `.vcxproj` / `.props`。
- 依赖管理：**vcpkg manifest** 或 **CMake FetchContent**，让 glfw / glm / assimp / stb 走源管理，删掉 `include/` 与 `lib/` 下的 vendored 二进制。`glad` 因为是生成器产物，按 third_party 保留。
- **三平台跨平台**：Windows / Linux / macOS。
- 目标目录布局：

  ```
  src/         运行时代码
  assets/      模型、纹理（替换现 res/）
  shaders/     GLSL（替换现 res/shaders/）
  third_party/ 必须 vendored 的（如 glad）
  tests/       单元测试
  ```

- 编译产物不再进 git，改用 GitHub Releases 发布。

### 阶段 2：拆头文件 + 消全局

- `ghost.h` / `player.h` / `World.h` 的实现全部挪到对应 `.cpp`。删空的 `common/ghost.cpp`。
- 全局 `window` / `mouseInfo` / `window_width` / `window_height` 收进一个 `App`（或 `Engine`）实例。GLFW 回调通过 `glfwSetWindowUserPointer` 拿上下文，不再吃全局。
- 合并 `Shader::Shader` 与 `LoadShaders`，只留一份。编译/链接失败 `throw`，**不再 `getchar(); exit(-1)`**。

### 阶段 3：去线程化 + 主循环驱动 ⭐ **关键**

这是项目稳定性的最大隐患，必须解决。

- 删掉 `Ghost` 里所有 `std::thread` / `std::atomic<bool>` flag / `animationThreads` vector。
- 引入 `Ghost::update(float dt, const Player&)`，由主循环在每帧调用。所有"动画"（looming、capture 缩放、twinkling、runAway）改写为基于 `dt` 累加状态的纯函数。
- `MouseInfo::firstMouse`、`Camera::timer` 等隐式状态改为显式由主循环推动。
- 顺手修掉：`setAmbient` 的 NaN bug、`setOffset` 自赋值、`drawCollisionBoxes` 的 buffer size 错误（或干脆删掉，留给将来 ImGui debug overlay）。

### 阶段 4：游戏架构

- 引入简单的 `GameState` 状态机替换 `main` 里 5 个 bool：

  ```
  Loading → Countdown(5s) → Playing → (Won | Lost) → Closing
  ```

- 实体管理用 `std::vector<std::unique_ptr<Entity>>` + 统一 `update(dt)` / `render(ctx)` 接口（先不上完整 ECS）。
- **关卡数据外置**：把 `Model::getTheCollideBox` 里写死的 mesh 索引（柱子 `0~15`、墙 `33~35` 等）替换为 JSON/TOML 配置或在 obj 里用命名约定（如 `collider_*` 节点 + Assimp 节点遍历）。
- 资源路径相对 exe 解析（`std::filesystem` + `argv[0]`），不再依赖 cwd。

### 阶段 5：开源化收尾

- `.gitignore` 加 `build/`、`.vs/`、`*.dll`、`*.lib`、IDE 临时文件。
- `README.md` 重写：保留截图 / GIF；构建说明换成 CMake 三连；加 macOS/Linux 段落；标注 LICENSE。
- 可选：`CONTRIBUTING.md`、`.editorconfig`、`.clang-format`。
- **GitHub Actions CI**：三平台 build matrix；可选静态分析（clang-tidy / cppcheck）。
- ⚠️ **资源版权审查**：`res/model/{garage,ghost,vacuum,emoji,sphere}` 等模型/纹理来源需逐一确认是否允许公开/商用。**这是设为 Public 之前的硬性前置任务**，否则有版权风险。不能用的需要换或删。
- 顺手把 `Excutable/` → `Executable/`、`screenshoots/` → `screenshots/`，路径在 README 里也同步更新。

### 阶段 6+：未来路线（含鬼 AI 寻路）

> 本节仅供未来扩展参考；阶段 1–5 落地后再考虑。

**重新引入线程的唯一合理场景：鬼 AI 寻路。**
当前 `runAway` 是傻瓜级（朝玩家反方向直线 + AABB 撞墙夹紧），鬼很容易卡角。要把它升级到真寻路时，**线程才有正当理由回归**——这和阶段 3 删掉的"动画热路径上的线程误用"是性质完全不同的两件事，不要混为一谈。

- **算法**：A\* over navmesh，或先做简化版（grid + Dijkstra），后续再换 navmesh。
- **并发模型**：`JobQueue` + 少量 worker 线程；鬼在 `update(dt, player)` 里**仅在状态变化时**（路径过期 / 玩家位置位移超阈值）提交一个 `PathRequest`，立即拿到一个 future；后续帧若 future ready 就采用新路径，否则继续走旧路径。
- **必须遵守的约束**（写在这里防止重蹈覆辙）：
  1. worker 线程**绝不**调任何 GL / GLFW 函数。
  2. worker 线程**只读**地图（navmesh 是不可变只读数据）和**只写自己拥有**的 path 缓冲，不直接写 ghost 成员。
  3. 主线程从 future 里拷出结果，**主线程**负责把 path 应用到 ghost。
  4. 不再用 `detach()`，统一用 `std::jthread` 或者拥有清晰生命周期的 worker pool（析构时 `request_stop` + `join`）。
- **可量测**：寻路平均耗时、p99 帧时间在 worker on/off 时的对比；这恰好对应原 SOW 第 4 条要求的"measurable result"。

**同期可考虑但优先级更低的扩展**（每条都是完整的小特性）：

- **异步资源加载**：启动时用 `std::async` 并行解析 obj，主线程跑 loading 屏，结果回主线程上传 GPU。一次性使用，不是常驻线程。
- **屏幕录制 / GIF 导出**：framebuffer readback → worker 编码 PNG/GIF，主线程不卡。README 现成有 GIF，可以做成游戏内"按 F12 截屏 / F11 录制"。
- **网络多人合作**（最大规模扩展）：直接对应课程 5 个专题里的 Sockets，玩法上有 Phasmophobia / Lethal Company 这类同类型范本可参考。

## 6. 注意事项 / 不动什么

- **玩法不变**：5 秒冻结倒计时、5 米吸鬼有效距离、与鬼接触判负、全鬼被吸判胜——这些数值和规则在重构里保持。
- **资源文件 `res/` 内容不动**，只改加载方式（路径解析、目录改名）。
- **操作映射不变**：WASD / 鼠标 / 左键 / Space / Esc / P。
- 重构期间始终以"功能等价"为基线，每个阶段结束都应能跑通游戏。

## 7. 给 Claude 的工作约定

- **改 header-only 大类时**，先把对应 cpp 实现挪好再改逻辑，避免一次性 PR 太大。
- **碰到全局变量**，优先把它包进上下文对象，不要"反正都 extern 了再加一个"。
- **碰到 `std::thread`**，停下来——本项目所有"用线程做动画"的写法都需要改成主循环 update，不是补一个 mutex。
- **路径相关**改动要记得 `Excutable` / `screenshoots` 这两个拼写错的目录在 README 与代码里都有引用。
- **资源加载失败** 不要再用 `getchar(); exit(-1)`，抛异常或返回 `std::expected` / `optional`。
- 当前默认开发分支：`claude/refactor-game-project-wcJfx`。
