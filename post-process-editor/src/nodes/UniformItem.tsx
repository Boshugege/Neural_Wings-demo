import React from 'react';
import { useStore } from '../store';
import type { UniformValue, UniformType } from '../types';

interface Props {
  nodeId: string;
  uKey: string;
  value: UniformValue;
}

export const UniformItem: React.FC<Props> = ({ nodeId, uKey, value }) => {
  const updateUniformValue = useStore(s => s.updateUniformValue);
  const changeUniformType = useStore(s => s.changeUniformType);
  const removeUniform = useStore(s => s.removeUniform);
  const renameUniformKey = useStore(s => s.renameUniformKey);
  const [localKey, setLocalKey] = React.useState(uKey);

  const isArray = Array.isArray(value);
  const size = isArray ? value.length : 1;

  const handleComponentChange = (index: number, newVal: string) => {
    const num = parseFloat(newVal) || 0;
    if (!isArray) {
      updateUniformValue(nodeId, uKey, num);
    } else {
      const nextValue = [...value];
      nextValue[index] = num;
      updateUniformValue(nodeId, uKey, nextValue);
    }
  };

  return (
    <div style={{
      display: 'grid',
      // 固定列宽：名字 75px, 中间输入框自适应, 类型下拉 42px, 删除按钮 15px
      gridTemplateColumns: '75px 1fr 42px 15px',
      gap: '4px',
      alignItems: 'center',
      marginBottom: '6px',
      width: '100%' // 确保撑满父容器
    }}>
      <input
        className="nodrag"
        value={localKey}
        onChange={(e) => setLocalKey(e.target.value)} // 局部更新，不影响焦点
        onBlur={() => renameUniformKey(nodeId, uKey, localKey)} // 结束时更新 Store
        onKeyDown={(e) => e.key === 'Enter' && renameUniformKey(nodeId, uKey, localKey)}
        style={{
          background: '#1a1a1a', border: '1px solid #333', color: '#888',
          fontSize: '10px', outline: 'none', padding: '2px 4px'
        }}
      />

      <div style={{
        display: 'grid',
        gridTemplateColumns: `repeat(${size}, 1fr)`,
        gap: '2px',
      }}>
        {Array.from({ length: size }).map((_, i) => (
          <input
            key={i}
            className="nodrag"
            type="number"
            value={isArray ? value[i] : value}
            onChange={(e) => handleComponentChange(i, e.target.value)}
            style={{
              background: '#111', border: '1px solid #333', color: '#fff',
              fontSize: '10px', width: '100%', padding: '1px 0', textAlign: 'center',
              borderRadius: '2px', outline: 'none'
            }}
          />
        ))}
      </div>

      <select
        className="nodrag"
        value={isArray ? `vec${size}` : 'float'}
        onChange={(e) => changeUniformType(nodeId, uKey, e.target.value as UniformType)}
        style={{
          background: '#333', color: '#aaa', border: 'none',
          fontSize: '9px', borderRadius: '2px', outline: 'none', height: '16px',
          appearance: 'none', textAlign: 'center'
        }}
      >
        <option value="float">flt</option>
        <option value="vec2">v2</option>
        <option value="vec3">v3</option>
        <option value="vec4">v4</option>
      </select>

      <button className="nodrag" onClick={() => removeUniform(nodeId, uKey)}
        style={{ background: 'none', border: 'none', color: '#555', cursor: 'pointer', padding: 0 }}>
        ×
      </button>
    </div>
  );
};