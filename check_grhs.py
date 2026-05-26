import struct

with open('Recursos/init/graficos.ind', 'rb') as f:
    data = f.read()

version, numGrhs = struct.unpack_from('<II', data, 0)
offset = 8
grhs = {}

for i in range(numGrhs):
    if offset + 2 > len(data):
        break
    numFrames = struct.unpack_from('<H', data, offset)[0]
    offset += 2

    if numFrames == 1:
        if offset + 10 > len(data): break
        fileNum, srcX, srcY, w, h = struct.unpack_from('<HHHHH', data, offset)
        offset += 10
        grhs[i+1] = {'file': fileNum, 'x': srcX, 'y': srcY, 'w': w, 'h': h}
    elif 1 < numFrames < 500:
        if offset + numFrames*2 + 4 > len(data): break
        offset += numFrames * 2 + 4

print(f"GRHs parseados: {len(grhs)}")
print(f"\nPrimeros 20 GRHs estáticos:")
for gid, g in list(grhs.items())[:20]:
    print(f"  Grh{gid}: file={g['file']}, x={g['x']}, y={g['y']}, {g['w']}x{g['h']}")
