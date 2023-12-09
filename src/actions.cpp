#include "actions.h"

//----------------------------------------------------------------------
// Action
//----------------------------------------------------------------------
void Action::SetKeystroke(Keystroke keystroke) {
    m_keystroke = keystroke;
}

void Action::SetModifiedKeystroke(uint8_t modifier, Keystroke keystroke) {
    if (keystroke == KEYSTROKE_NONE)
        m_modifier_keystrokes.erase(modifier);
    m_modifier_keystrokes[modifier] = keystroke;
}

const Keystroke& Action::GetKeystroke() const {
    return m_keystroke;
}

const Keystroke& Action::GetModifiedKeystroke(uint8_t modifier) const {
    auto it = m_modifier_keystrokes.find(modifier);
    if (it == m_modifier_keystrokes.end())
        return KEYSTROKE_NONE;
    return it->second;
}

const std::map<uint8_t, Keystroke>& Action::GetModifiedKeystrokes() const {
    return m_modifier_keystrokes;
}

const Keystroke& Action::GetDesiredKeystroke(std::unordered_set<uint8_t> pressed_keys) const {
    for (auto pressed_key : pressed_keys) {
        auto it = m_modifier_keystrokes.find(pressed_key);
            if (it != m_modifier_keystrokes.end()) {
                return it->second;
            }
    }

    return m_keystroke;
}

//----------------------------------------------------------------------
// Actions
//----------------------------------------------------------------------

Actions::Actions(): m_button_actions(), m_encoder_actions(), m_pressed_buttons() {}

 Keystroke Actions::GetButtonKeystroke(uint8_t button) {
    wxCriticalSectionLocker locker(m_actions_critsect);
    return m_button_actions[button].GetDesiredKeystroke(m_pressed_buttons);
}

Keystroke Actions::GetEncoderKeystroke(uint8_t encoder, EncoderDirection direction) {
    wxCriticalSectionLocker locker(m_actions_critsect);
    return m_encoder_actions[encoder][direction].GetDesiredKeystroke(m_pressed_buttons);
}

void Actions::IssueButton(uint8_t button, bool pressed) {
    auto keystroke = GetButtonKeystroke(button);
    {
        wxCriticalSectionLocker locker(m_pressed_critsect);
        if (pressed) {
            m_pressed_buttons.insert(button);
        } else {
            m_pressed_buttons.erase(button);
        }
    }
    if (keystroke != KEYSTROKE_NONE) {
        issueKeystroke(keystroke, pressed);
    }
}

void Actions::LoadTemplate(const ActionsTemplate& actions_template) {
    wxCriticalSectionLocker locker(m_actions_critsect);
    m_button_actions = actions_template.button_actions;
    m_encoder_actions = actions_template.encoder_actions;
}

void Actions::IssueEncoder(uint8_t encoder, int8_t count) {
    auto direction = count > 0 ? ENCODER_CW : ENCODER_CCW;
    auto abs_count = count > 0 ? count : -count;
    auto keystroke = GetEncoderKeystroke(encoder, count > 0 ? ENCODER_CW : ENCODER_CCW);
    if (keystroke != KEYSTROKE_NONE) {
        for (int i = 0; i < abs_count; i++) {
            issueKeystroke(keystroke, true);
            issueKeystroke(keystroke, false);
        }
    }
}
