// @ts-expect-error - bala
import { Handle, Position } from 'reactflow';

export default function InputNode() {
    return (
        <div style={{
            background: '#1a3d2e', color: '#4ade80', padding: '10px 20px',
            borderRadius: '20px', border: '1px solid #2d6a4f', fontWeight: 'bold'
        }}>
            源：inScreen
            <Handle type="source" position={Position.Right} id="output" style={{ background: '#4ade80' }} />
            <div style={{ position: 'absolute', bottom: '-5px', left: '50%', transform: 'translateX(-50%)' }}>
                <Handle
                    type="sourceDepth"
                    position={Position.Bottom}
                    id="depth-out"
                    style={{ background: '#4ade80', width: '10px', height: '10px', border: '2px solid #2d2d2d' }}
                />
                <span style={{ fontSize: '7px', color: '#4ade80', bottom: '-5px', right: '-35px', position: 'absolute', textAlign: 'center' }}>DEPTH</span>

            </div>
        </div>

    );
}