#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Lyra相机系统思维导图生成器
生成Mermaid格式的流程图和思维导图
"""

import os
from datetime import datetime

class LyraCameraSystemMindMap:
    def __init__(self):
        self.components = {}
        self.workflows = {}
        self.initialize_components()
        self.initialize_workflows()
    
    def initialize_components(self):
        """初始化所有相机系统组件"""
        self.components = {
            "LyraPlayerCameraManager": {
                "role": "相机系统总指挥",
                "responsibilities": [
                    "管理相机视图目标(ViewTarget)",
                    "处理UI相机优先级",
                    "协调相机组件工作",
                    "提供调试信息"
                ],
                "key_methods": ["UpdateCamera", "SetViewTarget", "UpdateViewTarget"],
                "dependencies": ["LyraCameraComponent", "LyraUICameraManager"]
            },
            
            "LyraCameraComponent": {
                "role": "相机计算核心",
                "responsibilities": [
                    "管理相机模式栈",
                    "计算最终相机视图",
                    "处理相机防穿墙",
                    "协调角色控制器"
                ],
                "key_methods": ["GetCameraView", "UpdateCameraModes"],
                "dependencies": ["LyraCameraModeStack", "PenetrationAvoidance"]
            },
            
            "LyraCameraModeStack": {
                "role": "效果叠加管理器",
                "responsibilities": [
                    "管理多个相机模式",
                    "计算模式混合权重",
                    "处理模式生命周期",
                    "提供调试信息"
                ],
                "key_methods": ["PushCameraMode", "EvaluateStack", "UpdateStack", "BlendStack"],
                "dependencies": ["LyraCameraMode", "FLyraCameraModeView"]
            },
            
            "LyraCameraMode": {
                "role": "具体相机行为",
                "responsibilities": [
                    "定义特定相机行为",
                    "计算相机位置和旋转",
                    "处理相机过渡效果",
                    "提供相机视图数据"
                ],
                "key_methods": ["UpdateView", "UpdateBlending", "GetPivotLocation", "GetPivotRotation"],
                "dependencies": ["FLyraCameraModeView", "CameraAssistInterface"]
            },
            
            "FLyraCameraModeView": {
                "role": "相机视图数据容器",
                "responsibilities": [
                    "存储相机视图参数",
                    "提供视图混合功能",
                    "序列化相机状态"
                ],
                "key_methods": ["Blend"],
                "dependencies": []
            },
            
            "PenetrationAvoidance": {
                "role": "防穿墙系统",
                "responsibilities": [
                    "检测相机碰撞",
                    "避免相机穿墙",
                    "处理相机卡顿",
                    "优化检测性能"
                ],
                "key_methods": ["UpdatePreventPenetration"],
                "dependencies": ["FLyraPenetrationAvoidanceFeeler"]
            },
            
            "CameraAssistInterface": {
                "role": "特殊情境处理",
                "responsibilities": [
                    "提供特殊相机行为",
                    "处理相机穿透规则",
                    "通知相机事件",
                    "扩展相机功能"
                ],
                "key_methods": ["GetIgnoredActors", "GetPreventPenetrationTarget", "OnCameraPenetrating"],
                "dependencies": []
            }
        }
    
    def initialize_workflows(self):
        """初始化工作流程"""
        self.workflows = {
            "normal_update": [
                ("游戏主循环", "调用UpdateCamera"),
                ("LyraPlayerCameraManager", "检查UI相机优先级"),
                ("LyraPlayerCameraManager", "调用UpdateViewTarget"),
                ("LyraCameraComponent", "调用GetCameraView"),
                ("LyraCameraModeStack", "调用EvaluateStack"),
                ("LyraCameraModeStack", "调用UpdateStack更新所有模式"),
                ("LyraCameraModeStack", "调用BlendStack混合效果"),
                ("FLyraCameraModeView", "执行Blend混合计算"),
                ("PenetrationAvoidance", "执行防穿墙检测"),
                ("最终相机视图", "应用到游戏相机")
            ],
            
            "camera_mode_change": [
                ("游戏逻辑", "检测到状态变化"),
                ("游戏逻辑", "调用PushCameraMode"),
                ("LyraCameraModeStack", "创建/获取相机模式实例"),
                ("LyraCameraModeStack", "计算初始混合权重"),
                ("LyraCameraModeStack", "插入模式到栈顶"),
                ("LyraCameraMode", "开始混合过渡"),
                ("LyraCameraMode", "每帧更新混合进度"),
                ("LyraCameraModeStack", "自动清理过期模式")
            ],
            
            "penetration_avoidance": [
                ("LyraCameraComponent", "检测到需要防穿墙"),
                ("PenetrationAvoidance", "配置探测器阵列"),
                ("PenetrationAvoidance", "执行碰撞检测"),
                ("CameraAssistInterface", "提供特殊穿透规则"),
                ("PenetrationAvoidance", "计算避让位置"),
                ("LyraCameraComponent", "应用避让调整")
            ]
        }
    
    def generate_mermaid_flowchart(self):
        """生成Mermaid格式的流程图"""
        mermaid_code = """```mermaid
graph TD
    %% 主流程图 - Lyra相机系统协作关系
    
    %% 入口点
    A[游戏主循环] --> B[LyraPlayerCameraManager.UpdateCamera]
    
    %% 相机管理器流程
    B --> C{检查UI相机优先级}
    C -->|UI优先| D[使用UI相机]
    C -->|游戏优先| E[调用UpdateViewTarget]
    
    %% 相机组件流程
    E --> F[LyraCameraComponent.GetCameraView]
    F --> G[LyraCameraModeStack.EvaluateStack]
    
    %% 模式栈更新流程
    G --> H[UpdateStack更新模式]
    H --> I[遍历所有相机模式]
    I --> J[每个模式.UpdateCameraMode]
    J --> K[清理过期模式]
    
    %% 混合流程
    G --> L[BlendStack混合效果]
    L --> M[从栈底开始混合]
    M --> N[FLyraCameraModeView.Blend]
    N --> O[位置线性插值]
    N --> P[旋转角度差值]
    N --> Q[视野线性插值]
    
    %% 防穿墙流程
    F --> R[PenetrationAvoidance检测]
    R --> S[配置探测器阵列]
    S --> T[执行碰撞检测]
    T --> U[CameraAssistInterface咨询]
    U --> V[计算避让位置]
    
    %% 最终输出
    O --> W[最终相机位置]
    P --> X[最终相机旋转]
    Q --> Y[最终相机视野]
    V --> Z[防穿墙调整]
    
    W --> AA[应用到游戏相机]
    X --> AA
    Y --> AA
    Z --> AA
    
    %% 样式定义
    classDef manager fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    classDef component fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    classDef mode fill:#e8f5e8,stroke:#1b5e20,stroke-width:2px
    classDef data fill:#fff3e0,stroke:#e65100,stroke-width:2px
    classDef system fill:#fce4ec,stroke:#880e4f,stroke-width:2px
    
    class B,D manager
    class F,G component
    class H,I,J,K,L,M mode
    class N,O,P,Q data
    class R,S,T,U,V system
```"""
        return mermaid_code
    
    def generate_mermaid_mindmap(self):
        """生成Mermaid格式的思维导图"""
        mindmap_code = """```mermaid
mindmap
  root((Lyra相机系统))
    
    %% 核心组件
    核心组件
      相机管理器(LyraPlayerCameraManager)
        职责
          管理ViewTarget
          处理UI优先级
          协调组件工作
          提供调试信息
        关键方法
          UpdateCamera
          SetViewTarget
          UpdateViewTarget
      
      相机组件(LyraCameraComponent)
        职责
          管理模式栈
          计算最终视图
          处理防穿墙
          协调控制器
        关键方法
          GetCameraView
          UpdateCameraModes
      
      模式栈(LyraCameraModeStack)
        职责
          管理多个模式
          计算混合权重
          处理生命周期
          提供调试信息
        关键方法
          PushCameraMode
          EvaluateStack
          UpdateStack
          BlendStack
      
      相机模式(LyraCameraMode)
        职责
          定义相机行为
          计算位置旋转
          处理过渡效果
          提供视图数据
        关键方法
          UpdateView
          UpdateBlending
          GetPivotLocation
          GetPivotRotation
    
    %% 支持系统
    支持系统
      视图数据(FLyraCameraModeView)
        职责
          存储视图参数
          提供混合功能
          序列化状态
        关键方法
          Blend
      
      防穿墙(PenetrationAvoidance)
        职责
          检测相机碰撞
          避免相机穿墙
          处理相机卡顿
          优化检测性能
        关键方法
          UpdatePreventPenetration
      
      辅助接口(CameraAssistInterface)
        职责
          提供特殊行为
          处理穿透规则
          通知相机事件
          扩展相机功能
        关键方法
          GetIgnoredActors
          GetPreventPenetrationTarget
          OnCameraPenetrating
    
    %% 工作流程
    工作流程
      正常更新流程
        游戏主循环调用
        检查UI优先级
        更新ViewTarget
        计算相机视图
        混合模式效果
        应用防穿墙
        输出最终视图
      
      模式切换流程
        检测状态变化
        推入新模式
        计算初始权重
        开始混合过渡
        自动清理模式
      
      防穿墙流程
        配置探测器
        执行碰撞检测
        咨询辅助接口
        计算避让位置
        应用调整
```"""
        return mindmap_code
    
    def generate_component_relationships(self):
        """生成组件关系图"""
        relationship_code = """```mermaid
erDiagram
    %% Lyra相机系统组件关系图
    
    LyraPlayerCameraManager ||--o{ LyraCameraComponent : "包含"
    LyraPlayerCameraManager ||--o{ LyraUICameraManager : "管理"
    
    LyraCameraComponent ||--o{ LyraCameraModeStack : "拥有"
    LyraCameraComponent ||--o{ PenetrationAvoidance : "使用"
    
    LyraCameraModeStack ||--o{ LyraCameraMode : "管理"
    LyraCameraModeStack ||--|| FLyraCameraModeView : "生成"
    
    LyraCameraMode ||--|| FLyraCameraModeView : "产生"
    LyraCameraMode }|--|| CameraAssistInterface : "咨询"
    
    PenetrationAvoidance ||--o{ FLyraPenetrationAvoidanceFeeler : "配置"
    PenetrationAvoidance }|--|| CameraAssistInterface : "咨询"
    
    %% 组件属性
    LyraPlayerCameraManager {
        string 角色 "相机系统总指挥"
        string[] 职责
        string[] 关键方法
    }
    
    LyraCameraComponent {
        string 角色 "相机计算核心"
        string[] 职责
        string[] 关键方法
    }
    
    LyraCameraModeStack {
        string 角色 "效果叠加管理器"
        string[] 职责
        string[] 关键方法
    }
    
    LyraCameraMode {
        string 角色 "具体相机行为"
        string[] 职责
        string[] 关键方法
    }
    
    FLyraCameraModeView {
        string 角色 "相机视图数据容器"
        string[] 职责
        string[] 关键方法
    }
    
    PenetrationAvoidance {
        string 角色 "防穿墙系统"
        string[] 职责
        string[] 关键方法
    }
    
    CameraAssistInterface {
        string 角色 "特殊情境处理"
        string[] 职责
        string[] 关键方法
    }
```"""
        return relationship_code
    
    def generate_html_report(self):
        """生成完整的HTML报告"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        html_content = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Lyra相机系统架构分析</title>
    <script src="https://cdn.jsdelivr.net/npm/mermaid@10.6.1/dist/mermaid.min.js"></script>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .section {{ margin: 30px 0; padding: 20px; border: 1px solid #ddd; }}
        .code-block {{ background: #f5f5f5; padding: 15px; border-radius: 5px; }}
        h1, h2, h3 {{ color: #333; }}
        .component-table {{ width: 100%; border-collapse: collapse; }}
        .component-table th, .component-table td {{ border: 1px solid #ddd; padding: 8px; }}
        .component-table th {{ background: #f0f0f0; }}
    </style>
</head>
<body>
    <h1>Lyra相机系统架构分析报告</h1>
    <p><strong>生成时间:</strong> {timestamp}</p>
    
    <div class="section">
        <h2>1. 系统组件总览</h2>
        <table class="component-table">
            <tr>
                <th>组件名称</th>
                <th>角色</th>
                <th>主要职责</th>
                <th>关键方法</th>
            </tr>
            {"".join([f'''
            <tr>
                <td><strong>{name}</strong></td>
                <td>{comp['role']}</td>
                <td>{'<br>'.join(comp['responsibilities'])}</td>
                <td>{'<br>'.join(comp['key_methods'])}</td>
            </tr>''' for name, comp in self.components.items()])}
        </table>
    </div>
    
    <div class="section">
        <h2>2. 系统协作流程图</h2>
        <div class="mermaid">
{self.generate_mermaid_flowchart().replace('```mermaid', '').replace('```', '')}
        </div>
    </div>
    
    <div class="section">
        <h2>3. 组件关系思维导图</h2>
        <div class="mermaid">
{self.generate_mermaid_mindmap().replace('```mermaid', '').replace('```', '')}
        </div>
    </div>
    
    <div class="section">
        <h2>4. 组件依赖关系图</h2>
        <div class="mermaid">
{self.generate_component_relationships().replace('```mermaid', '').replace('```', '')}
        </div>
    </div>
    
    <div class="section">
        <h2>5. 核心工作流程</h2>
        <h3>正常更新流程</h3>
        <ol>
            {"".join([f'<li>{step[0]} - {step[1]}</li>' for step in self.workflows['normal_update']])}
        </ol>
        
        <h3>相机模式切换流程</h3>
        <ol>
            {"".join([f'<li>{step[0]} - {step[1]}</li>' for step in self.workflows['camera_mode_change']])}
        </ol>
        
        <h3>防穿墙处理流程</h3>
        <ol>
            {"".join([f'<li>{step[0]} - {step[1]}</li>' for step in self.workflows['penetration_avoidance']])}
        </ol>
    </div>
    
    <script>
        mermaid.initialize({{ startOnLoad: true, theme: 'default' }});
    </script>
</body>
</html>"""
        
        return html_content
    
    def save_report(self, output_dir="d:\\UE_Project\\AegisOdyssey\\"):
        """保存报告文件"""
        # 创建输出目录
        os.makedirs(output_dir, exist_ok=True)
        
        # 保存HTML报告
        html_path = os.path.join(output_dir, "LyraCameraSystem_Analysis.html")
        with open(html_path, 'w', encoding='utf-8') as f:
            f.write(self.generate_html_report())
        
        # 保存Mermaid代码文件
        mermaid_path = os.path.join(output_dir, "LyraCameraSystem_Mermaid.md")
        with open(mermaid_path, 'w', encoding='utf-8') as f:
            f.write("# Lyra相机系统Mermaid图表\n\n")
            f.write("## 1. 系统协作流程图\n")
            f.write(self.generate_mermaid_flowchart() + "\n\n")
            f.write("## 2. 思维导图\n")
            f.write(self.generate_mermaid_mindmap() + "\n\n")
            f.write("## 3. 组件关系图\n")
            f.write(self.generate_component_relationships() + "\n\n")
        
        print(f"报告已生成到: {html_path}")
        print(f"Mermaid代码已保存到: {mermaid_path}")
        return html_path, mermaid_path

def main():
    """主函数"""
    print("正在生成Lyra相机系统架构分析报告...")
    
    # 创建思维导图生成器
    mindmap = LyraCameraSystemMindMap()
    
    # 生成并保存报告
    html_path, mermaid_path = mindmap.save_report()
    
    print("\n=== Lyra相机系统协作关系总结 ===")
    print("\n1. 核心协作流程:")
    print("   游戏循环 → 相机管理器 → 相机组件 → 模式栈 → 最终视图")
    
    print("\n2. 组件分工:")
    for name, comp in mindmap.components.items():
        print(f"   • {name}: {comp['role']}")
    
    print("\n3. 关键协作点:")
    print("   • 相机模式栈: 管理多个效果的叠加混合")
    print("   • 防穿墙系统: 与辅助接口协作处理特殊情境")
    print("   • 视图混合: 使用数学公式确保平滑过渡")
    
    print(f"\n详细报告已保存到: {html_path}")
    print("可以使用浏览器打开查看交互式图表")

if __name__ == "__main__":
    main()