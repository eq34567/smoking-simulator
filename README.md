# 抽烟模拟器

**▶ 在线玩：<https://eq34567.github.io/smoking-simulator/>**（电脑浏览器打开，首次加载贴图约 11 MB）

第一人称的网页抽烟模拟器：吧台、烟盒、打火机、烟灰缸、手机，窗外是富士山夜景。three.js（r186）单文件实现，双手是蒙皮手模，由程序化关节驱动。

## 本地运行

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
- `index.html`、`.nojekyll` — GitHub Pages 入口（跳转到 `smoking-simulator.html`）
