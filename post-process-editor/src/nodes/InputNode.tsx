import { Handle, Position } from 'reactflow';

export default function InputNode() {
    return (
        <div style={{
            background: '#1a3d2e', color: '#4ade80', padding: '10px 20px',
            borderRadius: '20px', border: '1px solid #2d6a4f', fontWeight: 'bold'
        }}>
            源：inScreen
            <Handle type="source" position={Position.Right} id="output" style={{ background: '#4ade80' }} />
        </div>
    );
}