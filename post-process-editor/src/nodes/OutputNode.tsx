import { Handle, Position } from 'reactflow';

export default function OutputNode() {
    return (
        <div style={{
            background: '#3d1a1a', color: '#f87171', padding: '10px 20px',
            borderRadius: '20px', border: '1px solid #7f1d1d', fontWeight: 'bold'
        }}>
            <Handle type="target" position={Position.Left} id="input" style={{ background: '#f87171' }} />
            输出：outScreen
        </div>
    );
}