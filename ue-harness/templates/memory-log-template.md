# Memory Log Template

用途：每次任务结束后写短记忆，把本轮经验回流到下一轮。Memory 不是聊天记录，也不是流水账，而是下次会被读取的工程沉淀。

## 1. Memory Identity

```yaml
task: "<任务名>"
system: "<所属系统>"
date: "<YYYY-MM-DD>"
author_agent: "<主 agent 或记录者>"
source_adapter: "<对应 Project Adapter>"
```

## 2. 本轮最小交付

用 3 到 5 句话说明本轮实际完成了什么。

## 3. 新增稳定事实

只写半年后仍应成立的事实。

- 系统入口：
- 设计边界：
- 调用链事实：
- 验证入口：

## 4. 阶段性提醒

只写阶段性内容。若项目没有实体 `Notice` 目录，标记为 `Notice candidate`。

- 临时判断：
- 未完成事项：
- 历史背景：
- 交接提醒：

## 5. 踩坑与误判

最好写成“现象 -> 根因 -> 下次避免方式”。

- 现象：
- 根因：
- 下次避免：

## 6. 规则升级候选

- 应升级到 `Rule`：
- 应升级到 `Checklist`：
- 应升级到 `Script`：

## 7. 下一轮读取建议

下一次相关任务开始前应优先读取：

- Harness 文件：
- Adapter 文件：
- Knowledge 文件：
- 版本历史或日志：
