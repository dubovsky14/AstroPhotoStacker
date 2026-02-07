
#include "../../headers/TaskScheduler.hxx"

#include <wx/wx.h>
#include <wx/progdlg.h>

#include <atomic>

template <typename TaskFunction>
void run_task_with_progress_dialog( const std::string &window_title,
                                    const std::string &message_begin,
                                    const std::string &message_end,
                                    const std::atomic<int> &tasks_processed,
                                    const int tasks_total,
                                    TaskFunction task,
                                    const std::string &message_final_task = "",
                                    int refresh_rate_ms = 100) {

        const std::string initial_message = (tasks_total <= 0) ?
            (message_begin + " 0 " + message_end) :
            (message_begin + " 0 / " + std::to_string(tasks_total) + " " + message_end);

        wxProgressDialog progress_bar(window_title, initial_message, tasks_total, nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        progress_bar.Update(tasks_processed);

        AstroPhotoStacker::TaskScheduler task_scheduler({size_t(1)});
        task_scheduler.submit(task, {1});
        while (task_scheduler.get_tasks_remaining()) {
            if (tasks_total <= 0) {
                progress_bar.Update(tasks_processed, message_begin + " " + std::to_string(tasks_processed) + " " + message_end);
            }
            else if (tasks_processed < tasks_total) {
                progress_bar.Update(tasks_processed, message_begin + " " + std::to_string(tasks_processed) + " / " + std::to_string(tasks_total) + " " + message_end);
            }
            else {
                progress_bar.Update(tasks_total-1, message_final_task);
            }
            wxMilliSleep(refresh_rate_ms);
        }

        task_scheduler.wait_for_tasks();

        progress_bar.Close();
        progress_bar.Destroy();
}