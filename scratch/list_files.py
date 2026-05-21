import os

source_dir = r"c:\Users\bo\Desktop\git\VRGunSystem_plugin\GunSystemDevelopment\Plugins\VRModularWeaponSystem\Source\VRModularWeaponSystem"

files_info = []
for root, dirs, files in os.walk(source_dir):
    for file in files:
        if file.endswith(('.h', '.cpp')):
            path = os.path.join(root, file)
            size = os.path.getsize(path)
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = len(f.readlines())
            files_info.append((os.path.relpath(path, source_dir), lines, size))

# Sort by relpath
files_info.sort()
print(f"Total files: {len(files_info)}")
for rel_path, lines, size in files_info:
    print(f"{rel_path:<70} | Lines: {lines:<5} | Size: {size:<6} bytes")
