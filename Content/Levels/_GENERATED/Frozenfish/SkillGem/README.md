# Uncut Skill Gem 草模

这个目录里放的是一版可直接导入 Unreal 的低模草模：

- `SM_UncutSkillGem.obj`
- `SM_UncutSkillGem.mtl`

造型目标：

- 偏 `未铭刻技能石` 的厚实晶体轮廓
- 不绑定具体技能语义
- 外壳和内核分成两个材质区，方便在 UE 里做半透明壳体和发光内芯

建议导入设置：

- 勾选 `Import Materials`
- 勾选 `Generate Missing Collision`
- 如果需要烘焙静态光照，勾选 `Generate Lightmap UVs`
- 如尺寸不合适，优先在导入时统一缩放，不要先改顶点

建议材质方向：

- `M_GemShell`：半透明或薄壳石英感，粗糙度中等偏低
- `M_GemCore`：使用 `Emissive` 做青蓝色呼吸光

建议拾取表现：

- 轻微悬浮
- 慢速自转
- 底部一圈淡能量环
