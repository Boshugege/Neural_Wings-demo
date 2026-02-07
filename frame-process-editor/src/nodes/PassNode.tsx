import { Handle, Position } from 'reactflow';
import type { NodeProps } from 'reactflow';
import { useStore } from '../store';
import type { PassNodeData } from '../types';
import { UniformItem } from './UniformItem';
import React, { useCallback, useEffect } from 'react';


// 定义在 PassNode.tsx 外部或单独的文件中
const RTNameInput = ({ nodeId, value, disabled }: { nodeId: string, value: string, disabled: boolean }) => {
    const [localValue, setLocalValue] = React.useState(value);
    const { updateNodeData, isRTNameDuplicate } = useStore();

    // 同步外部值
    React.useEffect(() => {
        setLocalValue(value);
    }, [value]);

    const handleCommit = () => {
        if (disabled) return; // 禁用时不处理提交
        if (localValue === value) return;

        const check = isRTNameDuplicate(nodeId, localValue);
        if (check.error) {
            alert(check.msg);
            setLocalValue(value);
        } else {
            updateNodeData(nodeId, { output: localValue });
        }
    };

    return (
        <input
            className="nodrag"
            // 如果禁用，强制显示 "outScreen"，否则显示当前编辑的值
            value={disabled ? "outScreen" : localValue}
            disabled={disabled}
            onChange={(e) => setLocalValue(e.target.value)}
            onBlur={handleCommit}
            onKeyDown={(e) => e.key === 'Enter' && handleCommit()}
            style={{
                background: disabled ? '#222' : '#111',       // 禁用变深色
                border: `1px solid ${disabled ? '#333' : '#3d3d3d'}`,
                borderRadius: '4px',
                color: disabled ? '#555' : '#fff',            // 禁用变灰色
                fontSize: '11px',
                width: '90px',
                textAlign: 'right',
                padding: '2px 6px',
                outline: 'none',
                cursor: disabled ? 'not-allowed' : 'text'     // 禁用显示禁止符号
            }}
        />
    );
};

// 颜色转换工具函数
const rgbaToHex = (r: number, g: number, b: number) => {
    return "#" + [r, g, b].map(x =>
        Math.max(0, Math.min(255, Math.floor(x))).toString(16).padStart(2, '0')
    ).join('');
};

const hexToRgba = (hex: string, alpha: number): [number, number, number, number] => {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    return [r, g, b, alpha];
};
const isLightColor = (hex: string) => {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    const brightness = (r * 299 + g * 587 + b * 114) / 1000;
    return brightness > 155;
};
interface EditablePortLabelProps {
    value: string;
    onSave: (value: string) => void;
    style: React.CSSProperties;
}
const EditablePortLabel: React.FC<EditablePortLabelProps> = ({ value, onSave, style }) => {
    const [local, setLocal] = React.useState(value);
    React.useEffect(() => { setLocal(value); }, [value]);

    return (
        <input
            className="nodrag"
            value={local}
            onChange={(e) => setLocal(e.target.value)}
            onBlur={() => onSave(local)}
            onKeyDown={(e) => e.key === 'Enter' && onSave(local)}
            style={{ ...style, background: 'transparent', border: 'none', outline: 'none', width: '100%' }}
        />
    );
};
export default function PassNode({ id, data }: NodeProps<PassNodeData>) {

    // 从 Store 获取所有操作方法
    const renameInputPort = useStore((s) => s.renameInputPort);
    const disconnectHandle = useStore((s) => s.disconnectHandle);
    const removeInputPort = useStore((s) => s.removeInputPort);
    const addUniform = useStore((s) => s.addUniform);
    const updateBaseColor = useStore((s) => s.updateBaseColor);
    const { updateNodeData, renameTexturePort, removeTexturePort, setNodeThemeColor } = useStore();
    const theme = data.themeColor || '#3d3d3d';
    const dataColor = data.baseColor
        ? rgbaToHex(data.baseColor[0], data.baseColor[1], data.baseColor[2])
        : '#3d3d3d';
    // 颜色处理：确保 alpha 映射正确
    const currentColor = data.baseColor || [255, 255, 255, 255];
    const hexValue = rgbaToHex(currentColor[0], currentColor[1], currentColor[2]);
    // CSS rgba 使用 0-1 的 alpha
    const cssRgba = `rgba(${currentColor[0]}, ${currentColor[1]}, ${currentColor[2]}, ${currentColor[3] / 255})`;

    const isThemeLight = isLightColor(theme);
    const titleTextColor = isThemeLight ? '#000' : '#fff';
    const labelColor = '#999';
    const isFinalPass = useStore(useCallback((store) =>
        store.edges.some(e => e.source === id && e.target === 'END_NODE'),
        [id]));
    useEffect(() => {
        if (isFinalPass) {
            disconnectHandle(id, 'depth-in');
        }
    }, [isFinalPass, id, disconnectHandle]);


    return (
        <div className="pass-node" style={{
            background: '#2d2d2d', color: '#ccc', borderRadius: '6px', border: `1px solid ${theme}`,
            width: '300px', overflow: 'visible', boxShadow: `0 4px 15px ${theme}33`, fontSize: '11px', fontFamily: 'Inter, system-ui, sans-serif'
        }}>
            {/* 标题栏背景跟随主题色 */}
            <div style={{
                background: theme,
                padding: '4px 8px',
                display: 'flex',
                justifyContent: 'space-between',
                alignItems: 'center',
                borderTopLeftRadius: '5px',
                borderTopRightRadius: '5px'
            }}>
                <input className="nodrag" value={data.name}
                    onChange={(e) => updateNodeData(id, { name: e.target.value })}
                    style={{
                        background: 'transparent', border: 'none',
                        color: titleTextColor, // 动态颜色
                        fontWeight: 'bold', outline: 'none', width: '120px', fontSize: '12px'
                    }}
                />


                {/* 右侧两个选择器 */}
                <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>

                    {/* 1. Base Color 选择器 (Shader 数据) */}
                    <div style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                        <span style={{ fontSize: '9px', color: titleTextColor, opacity: 0.7 }}>DATA</span>
                        <div style={{ position: 'relative', width: '16px', height: '16px' }}>
                            <div style={{
                                width: '100%', height: '100%', borderRadius: '2px',
                                background: cssRgba, color: titleTextColor, border: '1px solid', pointerEvents: 'none'
                            }} />
                            <input type="color" className="nodrag" value={hexValue}
                                onChange={(e) => updateBaseColor(id, hexToRgba(e.target.value, currentColor[3]))}
                                style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%', opacity: 0, cursor: 'pointer' }}
                            />
                        </div>
                    </div>

                    {/* 2. Theme Color 选择器 (UI 视觉) */}
                    <div style={{ display: 'flex', alignItems: 'center', gap: '4px', borderLeft: '1px solid rgba(255,255,255,0.2)', paddingLeft: '8px' }}>
                        <span style={{ fontSize: '8px', color: titleTextColor }}>UI:</span>
                        <div style={{ position: 'relative', width: '14px', height: '14px' }}>
                            <div style={{
                                width: '100%', height: '100%', borderRadius: '50%', // 改为圆形区分
                                background: theme, color: titleTextColor, border: '1px solid', pointerEvents: 'none'
                            }} />
                            <input type="color" className="nodrag" value={theme}
                                onChange={(e) => setNodeThemeColor(id, e.target.value)}
                                style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%', opacity: 0, cursor: 'pointer' }}
                            />
                        </div>
                    </div>
                </div>
            </div>

            <div style={{ padding: '8px' }}>
                {/* 2. VS/FS 编辑区 - 提高文字亮度 */}
                <div style={{ display: 'flex', flexDirection: 'column', gap: '6px', marginBottom: '12px' }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                        <span style={{ fontSize: '10px', color: labelColor, width: '20px', fontWeight: 'bold' }}>VS</span>
                        <input className="nodrag" value={data.vs} placeholder="assets/shaders/postprocess/default.vs"
                            onChange={(e) => updateNodeData(id, { vs: e.target.value })}
                            style={{ background: '#1a1a1a', border: '1px solid #3d3d3d', color: '#ccc', padding: '3px 6px', borderRadius: '4px', fontSize: '10px', flex: 1, outline: 'none' }} />
                    </div>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                        <span style={{ fontSize: '10px', color: labelColor, width: '20px', fontWeight: 'bold' }}>FS</span>
                        <input className="nodrag" value={data.fs} placeholder="assets/shaders/postprocess/default.vs"
                            onChange={(e) => updateNodeData(id, { fs: e.target.value })}
                            style={{ background: '#1a1a1a', border: '1px solid #3d3d3d', color: '#ccc', padding: '3px 6px', borderRadius: '4px', fontSize: '10px', flex: 1, outline: 'none' }} />
                    </div>
                </div>

                {/* 3. Base Color 详情 - 这里的 RGBA 标签也调亮 */}
                <div style={{ fontSize: '9px', color: dataColor, fontWeight: 'bold', marginBottom: '6px', borderTop: '1px solid #3d3d3d', paddingTop: '8px', letterSpacing: '0.5px' }}>BASE COLOR</div>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '6px', marginBottom: '12px' }}>
                    {currentColor.map((val, i) => (
                        <div key={i} style={{ display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                            <span style={{ fontSize: '8px', color: '#666', marginBottom: '2px' }}>{['R', 'G', 'B', 'A'][i]}</span>
                            <input className="nodrag" type="number" value={val}
                                onChange={(e) => {
                                    const next = [...currentColor] as [number, number, number, number];
                                    next[i] = Math.min(255, Math.max(0, parseInt(e.target.value) || 0));
                                    updateBaseColor(id, next);
                                }}
                                style={{ background: '#1a1a1a', border: '1px solid #3d3d3d', color: '#fff', fontSize: '10px', width: '100%', textAlign: 'center', borderRadius: '3px', padding: '2px 0' }}
                            />
                        </div>
                    ))}
                </div>


                {/* 4. 输入端口 (INPUTS) */}
                <div style={{ fontSize: '9px', color: theme, fontWeight: 'bold', marginBottom: '6px', borderTop: '1px solid #3d3d3d', paddingTop: '8px' }}>INPUTS</div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '3px' }}>
                    {data.inputPorts?.map((port) => (
                        <div key={port.id} style={{ display: 'flex', alignItems: 'center', position: 'relative', height: '20px' }}>
                            {/* 关键：Handle 绑定永久 ID */}
                            <Handle
                                type="target"
                                position={Position.Left}
                                id={port.id}
                                style={{ background: '#aaa', width: '8px', height: '8px', left: '-12px', border: '2px solid #2d2d2d' }}
                            />
                            <div style={{ height: '20px', display: 'flex', alignItems: 'center', flex: 1, background: '#1a1a1a', borderRadius: '3px', border: '1px solid #333' }}>
                                <EditablePortLabel
                                    value={port.name} // 显示用户起的名字
                                    onSave={(newVal) => renameInputPort(id, port.id, newVal)} // 提交时传 port.id
                                    style={{ color: '#aaa', fontSize: '10px' }}
                                />
                                <button className="nodrag" onClick={() => removeInputPort(id, port.id)} style={{ background: 'none', border: 'none', color: '#555', cursor: 'pointer', fontSize: '12px', paddingRight: '4px' }}>×</button>
                            </div>
                        </div>
                    ))}
                    <button className="nodrag" onClick={() => {
                        const newPort = { id: `in_${Date.now()}`, name: `u_rt${data.inputPorts.length}` };
                        updateNodeData(id, { inputPorts: [...(data.inputPorts || []), newPort] });
                    }} style={{ fontSize: '9px', color: '#888', background: '#353535', border: '1px solid #444', borderRadius: '4px', padding: '2px' }}>
                        + Add Input
                    </button>
                </div>

                {/* 贴图插槽 (TEXTURES) */}
                <div style={{ fontSize: '9px', color: theme, fontWeight: 'bold', marginBottom: '6px', marginTop: '6px', borderTop: '1px solid #3d3d3d', paddingTop: '8px' }}>TEXTURES</div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '3px' }}>
                    {data.texturePorts?.map((port) => (
                        <div key={port.id} style={{ display: 'flex', alignItems: 'center', position: 'relative', height: '20px' }}>
                            <Handle
                                type="target"
                                position={Position.Left}
                                id={port.id}
                                style={{ background: '#3a8ee6', width: '8px', height: '8px', left: '-12px', border: '2px solid #2d2d2d' }}
                            />
                            <div style={{ display: 'flex', alignItems: 'center', flex: 1, background: '#1a1a1a', borderRadius: '3px', border: '1px solid #333', height: '20px' }}>
                                <EditablePortLabel
                                    value={port.name}
                                    onSave={(newVal) => renameTexturePort(id, port.id, newVal)}
                                    style={{ color: '#3a8ee6', fontSize: '10px' }}
                                />
                                <button className="nodrag" onClick={() => removeTexturePort(id, port.id)} style={{ background: 'none', border: 'none', color: '#555', cursor: 'pointer', fontSize: '12px', paddingRight: '4px' }}>×</button>
                            </div>
                        </div>
                    ))}
                    <button className="nodrag" onClick={() => {
                        const newPort = { id: `in_${Date.now()}`, name: `u_tex${data.texturePorts.length}` };
                        updateNodeData(id, { texturePorts: [...(data.texturePorts || []), newPort] });
                    }}
                        style={{ fontSize: '9px', color: '#888', background: '#353535', border: '1px solid #444', borderRadius: '4px', padding: '2px' }}>+ Add Texture Slot</button>
                </div>


                {/* 5. Uniforms */}
                <div style={{ fontSize: '9px', color: theme, fontWeight: 'bold', margin: '10px 0 6px', borderTop: '1px solid #3d3d3d', paddingTop: '8px' }}>UNIFORMS</div>
                <div style={{ display: 'flex', flexDirection: 'column' }}>
                    {Object.entries(data.uniforms || {}).map(([key, val]) => (
                        <UniformItem key={key} nodeId={id} uKey={key} value={val} />
                    ))}
                    <button className="nodrag" onClick={() => addUniform(id, `u_value${Object.keys(data.uniforms).length}`, 0.0)}
                        style={{ fontSize: '9px', color: '#888', background: '#353535', border: '1px solid #444', borderRadius: '4px', padding: '4px', marginTop: '4px' }}>
                        + Add Uniform
                    </button>
                </div>

                {/* 6. 输出端口 */}
                <div style={{ display: 'flex', justifyContent: 'flex-end', alignItems: 'center', position: 'relative', marginTop: '12px', borderTop: '1px solid #3d3d3d', paddingTop: '8px' }}>

                    {/* 标签 */}
                    <span style={{ fontSize: '10px', marginRight: '6px', color: isFinalPass ? '#666' : theme, fontWeight: 'bold' }}>
                        {isFinalPass ? 'OUT:' : 'RT:'}
                    </span>

                    {/* 使用自定义组件，传入 disabled 状态 */}
                    <RTNameInput
                        nodeId={id}
                        value={data.output}
                        disabled={isFinalPass}
                    />

                    <Handle
                        type="source"
                        position={Position.Right}
                        id="output"
                        style={{ background: theme, width: '10px', height: '10px', right: '-15px', border: '2px solid #2d2d2d' }}
                    />
                </div>




                {!isFinalPass ? (
                    <div style={{ position: 'absolute', bottom: '-5px', left: '50%', transform: 'translateX(-50%)' }}>
                        <Handle
                            type="target"
                            position={Position.Bottom}
                            id="depth-in"
                            style={{ background: theme, width: '10px', height: '10px', border: '2px solid #2d2d2d' }}
                        />
                        <span style={{
                            fontSize: '7px',
                            color: theme,
                            bottom: '-10px',
                            left: '14px', // 调整文字位置，使其不重叠
                            position: 'absolute',
                            whiteSpace: 'nowrap',
                            fontWeight: 'bold'
                        }}>
                            DEPTH
                        </span>
                    </div>
                ) : (
                    <div />
                )}
            </div>
        </div >
    );
}