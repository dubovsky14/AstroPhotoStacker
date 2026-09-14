#pragma once

#include "../headers/VideoWriterSER.h"
#include "../headers/InputFrame.h"


#include <vector>
#include <memory>
#include <atomic>
#include <string>

namespace AstroPhotoStacker {
    class FramesToSERVideoConvertor {
        public:
            FramesToSERVideoConvertor() = delete;

            FramesToSERVideoConvertor(const std::vector<InputFrame> &input_frames, float fps = -1);

            size_t get_frames_total()   const   {return m_input_frames.size();};

            const std::atomic<int>& get_tasks_processed() const {return m_tasks_processed;};

            void save_to_file(const std::string &output_address);

        private:
            std::vector<InputFrame>         m_input_frames;
            std::atomic<int>                m_tasks_processed = 0;
            float                           m_fps = -1;

    };
}