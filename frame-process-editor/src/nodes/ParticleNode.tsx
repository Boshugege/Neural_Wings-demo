import { Handle, Position } from 'reactflow';
import type { NodeProps } from 'reactflow';
import { useStore } from '../store';
import type { ParticleNodeData } from '../types';

import React, { useEffect } from 'react';
const isLightColor = (hex: string) => {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    const brightness = (r * 299 + g * 587 + b * 114) / 1000;
    return brightness > 155;
};
const ParticleNameInput = ({ nodeId, value }: { nodeId: string, value: string }) => {
    const [localValue, setLocalValue] = React.useState(value);
    const { updateNodeData, isRTNameDuplicate } = useStore();

    useEffect(() => { setLocalValue(value); }, [value]);

    const handleCommit = () => {
        if (localValue === value) return;

        // 调用刚刚修改过的校验函数
        const check = isRTNameDuplicate(nodeId, localValue);

        if (check.error) {
            alert(check.msg);
            setLocalValue(value); // 回退
        } else {
            // 粒子节点同时更新 name 和 output
            updateNodeData(nodeId, { name: localValue, output: localValue });
        }
    };

    return (
        <input
            className="nodrag"
            value={localValue}
            onChange={(e) => setLocalValue(e.target.value)}
            onBlur={handleCommit}
            onKeyDown={(e) => e.key === 'Enter' && handleCommit()}
            style={{
                background: '#111',
                border: '1px solid #444',
                color: '#fff',
                fontSize: '12px',
                width: '90%',
                outline: 'none',
                padding: '3px 8px',
                borderRadius: '15px',
                textAlign: 'center'
            }}
        />
    );
};
export default function ParticleNode({ id, data }: NodeProps<ParticleNodeData>) {
    const { updateNodeData, setNodeThemeColor } = useStore();
    const theme = data.themeColor || '#e67e22'; // 默认粒子橙
    const isThemeLight = isLightColor(theme);
    const textColor = isThemeLight ? '#000' : '#fff';

    return (
        <div
            className="particle-node"
            style={{

                background: '#2d2d2d',
                width: '200px',
                borderTopLeftRadius: '10px',
                borderTopRightRadius: '10px',
                border: `1px solid ${theme}`,
                boxShadow: `0 4px 15px ${theme}44`,
                overflow: 'visible'
            }}>
            {/* 标题栏：包含名字和颜色选择器 */}
            <div style={{
                background: theme,
                padding: '4px 12px',
                borderTopLeftRadius: '10px',
                borderTopRightRadius: '10px',
                display: 'flex',
                justifyContent: 'space-between',
                alignItems: 'center'
            }}>
                <input
                    className="nodrag"
                    value={data.name}
                    onChange={(e) => updateNodeData(id, { name: e.target.value })}
                    style={{
                        background: 'transparent', border: 'none', color: textColor,
                        fontSize: '11px', fontWeight: 'bold', outline: 'none', width: '120px'
                    }}
                />

                {/* 颜色修改按钮 (UI 取色) */}
                <div style={{ position: 'relative', width: '14px', height: '14px' }}>
                    <div style={{
                        width: '100%', height: '100%', borderRadius: '00%', // 改为圆形区分
                        background: theme, color: textColor, border: '1px solid', pointerEvents: 'none'
                    }} />
                    <input type="color" className="nodrag" value={theme}
                        onChange={(e) => setNodeThemeColor(id, e.target.value)}
                        style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%', opacity: 0, cursor: 'pointer' }}
                    />
                </div>
            </div>

            <div style={{ padding: '10px 15px' }}>
                <div style={{ fontSize: '8px', color: theme, fontWeight: 'bold', marginBottom: '4px' }}>RT OUTPUT</div>
                <ParticleNameInput nodeId={id} value={data.output} />
            </div>


            {/* 粒子输出圆点 */}
            <Handle
                type="source"
                position={Position.Right}
                id="output"
                style={{
                    background: theme, width: '10px', height: '10px', right: '-5px',
                    border: '2px solid #2d2d2d'
                }}
            />

            <div style={{ position: 'absolute', bottom: '-5px', left: '50%', transform: 'translateX(-50%)' }}>
                <Handle
                    type="target"
                    position={Position.Bottom}
                    id="depth-in"
                    style={{ background: theme, width: '10px', height: '10px', border: '2px solid #2d2d2d' }}
                />
                <span style={{ fontSize: '7px', color: theme, bottom: '-5px', right: '-35px', position: 'absolute', textAlign: 'center' }}>DEPTH</span>

            </div>
        </div>
    );
}