#include "effects/backends/au/aubackend.h"

#import <AVFAudio/AVFAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <QList>
#include <QString>
#include <memory>

#include "effects/backends/au/aubackend.h"
#include "effects/backends/au/aueffectprocessor.h"
#include "effects/backends/au/aumanifest.h"
#include "effects/defs.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AUBackend : public EffectsBackend {
  public:
    AUBackend() : m_componentsById([[NSDictionary alloc] init]), m_nextId(0) {
        loadAudioUnits();
    }

    ~AUBackend() override {
    }

    EffectBackendType getType() const override {
        return EffectBackendType::AU;
    };

    const QList<QString> getEffectIds() const override {
        QList<QString> effectIds;

        for (NSString* effectId in m_componentsById) {
            effectIds.append(QString::fromNSString(effectId));
        }

        return effectIds;
    }

    EffectManifestPointer getManifest(const QString& effectId) const override {
        AVAudioUnitComponent* component =
                m_componentsById[effectId.toNSString()];
        return EffectManifestPointer(new AUManifest(
                effectId, QString::fromNSString([component name])));
    }

    const QList<EffectManifestPointer> getManifests() const override {
        QList<EffectManifestPointer> manifests;

        for (NSString* effectId in m_componentsById) {
            AVAudioUnitComponent* component = m_componentsById[effectId];
            manifests.append(EffectManifestPointer(
                    new AUManifest(QString::fromNSString(effectId),
                            QString::fromNSString([component name]))));
        }

        return manifests;
    }

    bool canInstantiateEffect(const QString& effectId) const override {
        return [m_componentsById objectForKey:effectId.toNSString()] != nil;
    }

    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override {
        return std::make_unique<AUEffectProcessor>(pManifest);
    }

  private:
    NSDictionary<NSString*, AVAudioUnitComponent*>* m_componentsById;
    int m_nextId;

    QString freshId() {
        return QString::number(m_nextId++);
    }

    void loadAudioUnits() {
        // See
        // https://developer.apple.com/documentation/audiotoolbox/audio_unit_v3_plug-ins/incorporating_audio_effects_and_instruments?language=objc

        // Create a query for audio components
        AudioComponentDescription description = {
                .componentType = kAudioUnitType_Effect,
                .componentSubType = 0,
                .componentManufacturer = 0,
                .componentFlags = 0,
                .componentFlagsMask = 0,
        };

        // Find the audio units
        // TODO: Should we perform this asynchronously (e.g. using Qt's
        // threading or GCD)?
        auto manager =
                [AVAudioUnitComponentManager sharedAudioUnitComponentManager];
        auto components = [manager componentsMatchingDescription:description];

        // Assign ids to the components
        NSMutableDictionary<NSString*, AVAudioUnitComponent*>* componentsById =
                [[NSMutableDictionary alloc] init];

        for (AVAudioUnitComponent* component in components) {
            QString effectId = freshId();
            componentsById[effectId.toNSString()] = component;
        }

        m_componentsById = componentsById;
    }
};

EffectsBackendPointer createAUBackend() {
    return EffectsBackendPointer(new AUBackend());
}
