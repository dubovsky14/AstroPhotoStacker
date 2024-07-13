
#include "../../headers/thread_pool.h"

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

        wxProgressDialog progress_bar(window_title, message_begin + " 0 / " + std::to_string(tasks_total) + " " + message_end, tasks_total, nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
        progress_bar.Update(tasks_processed);

        thread_pool pool(1);
        pool.submit(task);

        while (pool.get_tasks_total()) {

            if (tasks_processed < tasks_total) {
                progress_bar.Update(tasks_processed, message_begin + " " + std::to_string(tasks_processed) + " / " + std::to_string(tasks_total) + " " + message_end);
            }
            else {
                progress_bar.Update(tasks_total-1, message_final_task);
            }
            wxMilliSleep(refresh_rate_ms);
        }
        pool.wait_for_tasks();

        progress_bar.Close();
        progress_bar.Destroy();
}