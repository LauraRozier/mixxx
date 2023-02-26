#pragma once

#include <util/assert.h>

#include <QBoxLayout>
#include <QDebug>
#include <QDomElement>
#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <QtGlobal>
#include <memory>
#include <vector>

#include "defs_urls.h"
#include "preferences/usersettings.h"

#define MIN_SCREEN_SIZE_FOR_CONTROLLER_SETTING_ROW 960

class AbstractLegacyControllerSetting;

class LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutElement() {
    }
    virtual ~LegacyControllerSettingsLayoutElement() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const = 0;

    virtual QWidget* build(QWidget* parent) = 0;
};

class LegacyControllerSettingsLayoutContainer : public LegacyControllerSettingsLayoutElement {
  public:
    enum Disposition {
        HORIZONTAL = 0,
        VERTICAL,
    };

    LegacyControllerSettingsLayoutContainer(
            Disposition disposition = HORIZONTAL,
            Disposition widgetOrientation = HORIZONTAL)
            : LegacyControllerSettingsLayoutElement(),
              m_disposition(disposition),
              m_widgetOrientation(widgetOrientation) {
    }
    LegacyControllerSettingsLayoutContainer(const LegacyControllerSettingsLayoutContainer& other) {
        m_elements.reserve(other.m_elements.size());
        for (const auto& e : other.m_elements)
            m_elements.push_back(e->clone());
    }
    virtual ~LegacyControllerSettingsLayoutContainer() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutContainer>(*this);
    }

    void addItem(std::shared_ptr<AbstractLegacyControllerSetting> setting);
    void addItem(std::unique_ptr<LegacyControllerSettingsLayoutContainer>&& container) {
        m_elements.push_back(std::move(container));
    }

    virtual QWidget* build(QWidget* parent) override;

  protected:
    QBoxLayout* buildLayout(QWidget* parent) const;

    Disposition m_disposition;
    Disposition m_widgetOrientation;
    std::vector<std::unique_ptr<LegacyControllerSettingsLayoutElement>> m_elements;
};

class LegacyControllerSettingsGroup : public LegacyControllerSettingsLayoutContainer {
  public:
    LegacyControllerSettingsGroup(const QString& label,
            LegacyControllerSettingsLayoutContainer::Disposition disposition =
                    VERTICAL)
            : LegacyControllerSettingsLayoutContainer(disposition),
              m_label(label) {
    }
    virtual ~LegacyControllerSettingsGroup() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsGroup>(*this);
    }

    QWidget* build(QWidget* parent) override;
    
  private:
    QString m_label;
};

class LegacyControllerSettingsLayoutItem : public LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutItem(
            std::shared_ptr<AbstractLegacyControllerSetting> setting,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsGroup::HORIZONTAL)
            : LegacyControllerSettingsLayoutElement(),
              m_setting(setting),
              m_prefferedOrientation(orientation) {
    }
    virtual ~LegacyControllerSettingsLayoutItem() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutItem>(*this);
    }

    QWidget* build(QWidget* parent) override;

  private:
    std::shared_ptr<AbstractLegacyControllerSetting> m_setting;
    LegacyControllerSettingsLayoutContainer::Disposition m_prefferedOrientation;
};

class WLegacyControllerSettingsContainer : public QWidget {
    Q_OBJECT
  public:
    WLegacyControllerSettingsContainer(
            LegacyControllerSettingsLayoutContainer::Disposition
                    prefferedOrientation,
            QWidget* parent)
            : QWidget(parent), m_prefferedOrientation(prefferedOrientation) {
    }

  protected:
    void resizeEvent(QResizeEvent* event);

  signals:
    void orientationChanged(LegacyControllerSettingsLayoutContainer::Disposition);

  private:
    LegacyControllerSettingsLayoutContainer::Disposition m_prefferedOrientation;
};
