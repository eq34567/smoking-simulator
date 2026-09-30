# 抽烟模拟器

第一人称的网页抽烟模拟器：吧台、烟盒、打火机、烟灰缸、手机，窗外是富士山夜景。three.js（r186）单文件实现，双手是蒙皮手模，由程序化关节驱动。

> **目前只有网页版可以玩。虚幻引擎（Unreal Engine）版还没做**，只有一份计划和代码草稿，见文末[「虚幻展望」](#虚幻展望未完成)。

## 运行

浏览器不允许 `file://` 读取手模和贴图，需要本地起一个小服务器（需要 [Node.js](https://nodejs.org/)）：

```
node serve.js
```

然后打开 <http://localhost:8765>。Windows 下也可以直接双击 `启动网页版.bat`。

## 操作

| 按键 | 动作 |
|---|---|
| 按住空格 | 吸 |
| E | 抽一支 / 点火 |
| F | 弹烟灰 |
| X | 摁灭 |
| B | 吃颗槟榔 |
| P | 刷手机 |
| O | 开窗 |

界面底部的按钮也可以操作，另外还有「烟头漂浮」「梦核」风格和声音开关。

## 文件

- `smoking-simulator.html` — 整个网页版（HTML + ES module，three.js 通过 importmap 从 CDN 加载）
- `assets/hand/` — 左右手 GLB 模型
- `assets/tex/` — 木纹、布料、皮革 PBR 贴图
- `serve.js`、`启动网页版.bat` — 本地静态服务器
- `虚幻展望.zip` — 虚幻引擎版的计划和代码草稿（C++、材质、场景脚本）。**未完成，还没在虚幻引擎里做出来**

---

## 虚幻展望（未完成）

> ⚠️ **虚幻引擎版还没有做。** 下面是之前为它写的第一阶段（M1 静帧）计划和操作步骤。`虚幻展望.zip` 里的代码和脚本只是草稿，还没在引擎里实际搭出来，留作以后的方向。
>
> 下文提到的 `Config/`、`Source/`、`Materials/`、`Scripts/` 都在 `虚幻展望.zip` 解压后的 `UE_M1/` 目录里。

目标：一张静止画面以假乱真——吧台上放着烟盒和烟灰缸，你的右手夹着一支点燃的烟，窗外是夜景远山。
不做任何动作，只追求"截一张图像照片"。预计一周。

### 1. 建项目（约 30 分钟）

1. 新建 **C++ 空白项目**，名字建议 `SmokeSim`（不叫这个也行，见第 2 步）。
2. Edit → Plugins，确认开启：Niagara、Control Rig、Full Body IK、MetaHuman、MetaSounds、Enhanced Input、**Python Editor Script Plugin**、HDRIBackdrop。
3. 把 `Config/DefaultEngine_Renderer.ini` 的内容合并进项目的 `Config/DefaultEngine.ini`，重启编辑器（着色器会重新编译一段时间）。

### 2. 加入香烟类（约 30 分钟）

1. 把 `Source/Cigarette.h`、`Source/Cigarette.cpp` 放进 `Source/<你的模块名>/`。
2. 把两个文件里的 `SMOKESIM_API` 换成你的模块宏（项目叫 SmokeSim 就不用换）。
3. 在 `<模块名>.Build.cs` 的 `PublicDependencyModuleNames` 里加上 `"Niagara"`。
4. 编译。编辑器里 Content Browser 右键 → Blueprint Class → 搜 `Cigarette`，建 `BP_Cigarette`。

### 3. 搭场景（约 5 分钟）

1. 新建一个空关卡（Basic 或 Empty Level 都行）。
2. Tools → Execute Python Script → 选 `Scripts/M1_SceneSetup.py`。
3. Outliner 里会出现一个 `M1` 文件夹：吧台、玻璃幕墙、窗框、吊灯、月光、天空光、雾、后期、参考相机，以及几个 `Place_*` 标记点。
4. 视口里右键 `FP_EyeCamera_Reference` → Pilot，就是玩家的视角。

### 4. 做三个材质（约半天）

按 `Materials/M_CigPaper_BurnMask.hlsl` 文件里的说明做：

- **M_CigPaper**：Masked、双面。Custom 节点粘贴文件开头那段 HLSL，按注释连 BaseColor / Emissive / OpacityMask。参数名 `BurnFront`、`CigAxis`、`Glow`、`Lit` 必须一致，C++ 每帧会写。
- **M_CigEmber**：烟头那个发光小圆片，用缓慢平移的云雾噪声做发光。
- **M_CigAsh**：灰白、带层纹、靠近火头一端偏黑。
- 滤嘴材质：橙色软木斑点纹理（Megascans 里搜 cork 可以直接用）。

在 `BP_Cigarette` 里把四个材质填进 *Cigarette | Materials*，拖进场景。在 Details 里把 `Burn Progress` 调到 0.3 左右、`Ash Length` 调到 1.2 左右预览，在 Begin Play 里调用 `Light`，点击 Play 看烟头发光。

### 5. 换上真实素材（1–2 天）

通过 Fab / Quixel Bridge 导入 Megascans，摆到 `Place_*` 标记点上，替换灰盒：

| 灰盒 | 替换为 |
| --- | --- |
| Counter_Top | 胡桃木或深色橡木材质，加一层粗糙度约 0.3 的清漆 |
| Place_Ashtray | 玻璃或金属烟灰缸（没有合适的就用金属材质自己建） |
| Place_CigPack | 自己做的翻盖烟盒，用原创包装 |
| Mullion / Rail | 深色拉丝金属 |
| Glass | 半透明玻璃材质 + 一张很淡的污渍贴图 |

窗外：放一个 **HDRI Backdrop**，用 Poly Haven 的城市夜景 HDRI；远山用 Megascans 山体扫描或 Landscape，放到 3–5 km 外，交给雾处理。

### 6. MetaHuman 的手（2–3 天，M1 最关键的一步）

1. 在 MetaHuman Creator 做一个男性角色：手偏粗、指关节明显、肤色自然。通过 Bridge 导入。
2. 把角色放进场景，调整位置，让右手腕落在 `Place_RightHandRest` 附近，身体在相机下方（头部可以隐藏）。
3. 新建一个 Level Sequence，把 MetaHuman 加进去，使用它自带的 Body Control Rig，把右手摆成**夹烟姿势**：
   - 食指、中指伸展、微弯，夹住烟的位置在两指第一节和第二节之间。
   - 无名指、小指自然向掌心弯曲，拇指放松搭在侧面。
   - 手腕略向上抬，烟朝右前方斜向上。
4. 在右手 `middle_01_r` 骨骼上加一个 Socket，把 `BP_Cigarette` 挂上去，微调到正好夹在两指之间。
5. 这个姿势存成 Pose Asset，M3 做动画时直接复用。

### 7. 调光和出图（半天）

- 吊灯是主光：只打亮桌面中心和手，边缘自然暗下去。
- 月光很弱，只勾出山脊和窗框的冷色轮廓。
- 手动曝光 `auto_exposure_bias` 在 9–12 之间调，调到桌面木纹清楚、烟头发光不过曝。
- 用参考相机（24mm，焦点 45cm）截图，对照下面的清单验收。

### M1 验收清单

- [ ] 手：皮肤有毛孔和关节褶皱，明暗交界处透出暖红，指甲有高光。
- [ ] 香烟：卷纸有纤维感，燃烧处有一圈焦黑，烟头是斑驳流动的橙红光，不是一整块均匀发光。
- [ ] 烟头的光照在夹烟的两根手指上，看得出来。
- [ ] 桌面清漆映出吊灯的高光，烟灰缸有正常的金属或玻璃反射。
- [ ] 玻璃上能隐约看到室内灯的倒影，外面是有纵深的夜景，远山越远越淡越蓝。
- [ ] 截一张图发给没看过的人，对方第一反应不是"这是游戏截图"。

M1 过了，再做 M2（烟丝和吐烟）。
