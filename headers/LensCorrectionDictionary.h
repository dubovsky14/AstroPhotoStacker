#pragma once

#include "../headers/LensCorrectionTool.h"
#include "../headers/InputFrame.h"

#include <memory>
#include <string>
#include <map>

namespace AstroPhotoStacker {
    /**
     * LensCorrectionDictionary singleton class that stores lens correction tools for different lenses.
     */
    class LensCorrectionDictionary {
        public:
            static LensCorrectionDictionary& get_instance() {
                static LensCorrectionDictionary instance;
                return instance;
            };

            LensCorrectionDictionary(const LensCorrectionDictionary&) = delete;
            LensCorrectionDictionary& operator=(const LensCorrectionDictionary&) = delete;

            void add_corrections(const InputFrame& frame, std::shared_ptr<const LensCorrectionTool> correction) {
                m_corrections_map[frame] = correction;
            };

            void add_corrections(const InputFrame& frame, const std::string &correction_summary_string) {
                if (m_corrections_summary_string_to_correction_tool_map.find(correction_summary_string) == m_corrections_summary_string_to_correction_tool_map.end()) {
                    m_corrections_summary_string_to_correction_tool_map[correction_summary_string] = std::make_shared<LensCorrectionTool>(correction_summary_string);
                }
                m_corrections_map[frame] = m_corrections_summary_string_to_correction_tool_map[correction_summary_string];
            };

            std::shared_ptr<const LensCorrectionTool> get_lens_correction_tool(const InputFrame& frame) const {
                auto it = m_corrections_map.find(frame);
                if (it != m_corrections_map.end()) {
                    return it->second;
                }
                return nullptr;
            };

        private:
            LensCorrectionDictionary() = default;
            ~LensCorrectionDictionary() = default;

            std::map<InputFrame, std::shared_ptr<const LensCorrectionTool>> m_corrections_map;
            std::map<std::string, std::shared_ptr<const LensCorrectionTool>> m_corrections_summary_string_to_correction_tool_map; // to avoid creating multiple LensCorrectionTool instances for the same summary string
    };
}