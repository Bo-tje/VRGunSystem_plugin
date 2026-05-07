import os
import re

base_path = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\GunSystemDevelopment\Plugins\VRModularWeaponSystem\Source\VRModularWeaponSystem\Public'

for root, dirs, files in os.walk(base_path):
    for file in files:
        if file.endswith('.h'):
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Simple heuristic: let's just print class names and new functions/properties
            # To avoid huge output, just print the UCLASS/USTRUCT names and their file paths
            
            class_names = []
            for match in re.finditer(r'(?:class|struct)\s+(?:[A-Z0-9_]+_API\s+)?([A-Z]\w+)', content):
                name = match.group(1)
                if not name.endswith('API'):
                    class_names.append(name)
            
            if class_names:
                print(f"{os.path.relpath(filepath, base_path)}: {class_names[0]}")
