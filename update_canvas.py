import json

file_path = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\minor skilled notes\System design\System design chart (in-development).canvas'

with open(file_path, 'r', encoding='utf-8') as f:
    data = json.load(f)

node_updates = {
    'c8420321cae2a996': 'Actor \\nAVRWeaponBase',
    '2b64f0ac613b4955': 'Base Weapon class, container for modular components. Routes VR interaction and manages initialization.',
    'e56227df0ccaefed': 'Functions\\nInitializeWeapon()\\nApplyWeaponDataVisuals()\\nPrimaryAction()\\nPullTrigger()\\nReleaseTrigger()',
    '0bd1f312b012235a': 'Variables\\nUVRWeaponData* WeaponData\\nUBoxComponent* WeaponRoot\\nUSceneComponent* PartRoot\\nUVRWeaponStateTreeComponent* StateTreeComponent',
    'b30dc8db0b34fee9': 'StateTree Component / Logic',
    '50e633a30cda71e5': 'Evaluator Outputs:\\n- IsTriggerPulled\\n- HasRoundReady\\n- ChamberStateTag',
    'f6ae8129756d2e15': 'State Tree Tasks',
    '435b7786bed4512e': 'FSTTask_FireWeapon\\nFSTTask_EjectRound\\nFSTTask_ChamberRound',
    '717db59542f3dda8': 'Scene component\\nUVRFireComponent',
    'c97f750c6b910fdb': 'Handles the actual firing logic (hitscan / projectile spawning).',
    '13945775b939bb74': 'Functions\\nHandleFiring(UProjectileData* ProjectileData)\\nHandleDryFire()\\nGetMuzzleTransform()',
    '6224fdec14c86ca6': 'Variables\\nFName MuzzleSocketName\\nloat FireHapticScale\\nloat DryFireHapticScale',
    '97ab82e823fe63ac': 'Scene component\\nUVRChamberComponent',
    'df93535bdf916db6': 'Manages the round currently in the firing position.',
    '7d7deb0c25dc9f31': 'Functions\\nTryLoad(UProjectileData* NewRound)\\nTryEject()\\nTryGiveBullet()\\nIsRoundReady()\\nIsEmpty()',
    'e9baedc1e825a269': 'Variables\\nFGameplayTag CurrentChamberState\\nUProjectileData* LoadedProjectile',
    '45db76089280f200': 'Scene component\\nUVRGrabComponent',
    'a39d99826fb8e035': 'Component placed on grabbable actors',
    '92a6ab7126af1cfb': 'Functions\\nTryGrab(UVRInteractor* Interactor)\\nTryRelease()\\nIsHeld()\\nGetHoldingHand()',
    '6fc2070b89889c54': 'Variables\\nool bIsHeld, ool bUseSocketSnap, FName GrabSocketName\\nloat ThrowMultiplier\\nUHapticFeedbackEffect_Base* GrabHapticEffect\\nloat HapticScale, ool bLoopHaptics',
    '5832413d8c6129b8': 'Scene component\\nUVRInteractor',
    '8d924adbe5a285d0': 'Component placed on your hands',
    '42a72229ac698269': 'Functions\\nIntendGrab()\\nIntendRelease()\\nIntendActionStart(float Value, FGameplayTag ActionTag)\\nIntendActionStop(FGameplayTag ActionTag)\\nPlayHapticFeedback()\\nRequestRelease()',
    'ea30d22a7aeff379': 'Variables\\nEControllerHand HandSide\\nUVRGrabComponent* ActiveGrabComponent\\nUHapticFeedbackEffect_Base* HoverHapticEffect\\nUSphereComponent* DetectionSphereComponent\\nTWeakObjectPtr<UVRGrabComponent> HoverTarget',
    '5398f476a495e435': 'Data asset\\nUVRWeaponData',
    '78b86d2e97d86133': 'Defines the base configuration for a weapon',
    '2ec37b8d730db980': 'Variables:\\nloat FireRate, ool bUseHitscan, loat RecoilAmount\\nFGameplayTag CompatibleMagazinesTag\\nTArray<FVRWeaponPart> WeaponParts\\nUProjectileData* DefaultProjectile\\nUSoundBase* FireSound, UParticleSystem* MuzzleFlash',
    'fea3a6a9c75a8e60': 'Data asset\\nUProjectileData',
    'f96de082989d162f': 'Defines the properties of a bullet/round.',
    '54f2b87fd3aaa859': 'Variables\\nFGameplayTagContainer AmmoTags\\nloat Damage, loat HitscanDamage\\nloat InitialSpeed, loat GravityScale\\nTSubclassOf<AActor> ProjectileClass\\nloat HitscanRange\\nUStaticMesh* LiveRoundMesh, UStaticMesh* SpentCasingMesh',
    'a48d0306dc355dcb': 'Interface\\nIVRInteractableInterface',
    '287a104ca87bd0a0': 'Functions\\noid OnHoverStart(UObject* Interactor)\\noid OnHoverEnd(UObject* Interactor)\\noid StartAction(UObject* Interactor, float ActionValue, FGameplayTag ActionTag)\\noid StopAction(UObject* Interactor, FGameplayTag ActionTag)',
    'e763c5f473ed1b05': 'Interface\\nIVRInteractorInterface',
    '7f36b295c8a6b5cd': 'Functions\\nirtual APlayerController* GetProvidingPlayerController() const = 0;',
    'f66551a70d275cfc': None, 
    '18becb8fed66c70f': None   
}

for node in data.get('nodes', []):
    if node['id'] in node_updates and node_updates[node['id']] is not None:
        node['text'] = node_updates[node['id']]

with open(file_path, 'w', encoding='utf-8') as f:
    json.dump(data, f, indent=4)
