#include "test.hh"
#include <chrono>
#include <limits>
#include <map>

#if 0 // to run faster for develop
#define GOPTX_TEST_NO_PROFILE
#elif 0
#define GOPTX_TEST_PROFILE_FULL
#endif

#ifndef GOPTX_TEST_NO_PROFILE

#include <wuk/cupti_wrapper.hh>

#endif

namespace GoPTX {

namespace test {

std::string metric(const std::string &prepend,
                   const std::function<void()> &reset,
                   const std::function<void()> &kernel, int test_time) {
  std::string ret;
  ret = prepend + ret;
  std::vector<float> times;
  for (int i = 0; i < test_time; ++i) {
    reset();
    auto beg = std::chrono::high_resolution_clock::now();
    kernel();
    auto end = std::chrono::high_resolution_clock::now();
    times.push_back(
        std::chrono::duration<float, std::milli>(end - beg).count());
  }
  float min_time = std::numeric_limits<float>::max(),
        max_time = std::numeric_limits<float>::min(), avg_time = 0;
  for (auto time : times) {
    if (min_time > time)
      min_time = time;
    if (max_time < time)
      max_time = time;
    avg_time += time;
  }
  avg_time /= times.size();
  ret += ", \"min(ms)\": " + std::to_string(min_time) +
         ", \"max(ms)\": " + std::to_string(max_time) +
         ", \"avg(ms)\": " + std::to_string(avg_time);
#ifndef GOPTX_TEST_NO_PROFILE
  do {
    // https://docs.nvidia.com/cupti/main/main.html#metrics-mapping-table
    std::vector<std::string> metricNames {
#ifdef GOPTX_TEST_PROFILE_FULL
      "dram__bytes_read.sum.per_second", "dram__bytes_write.sum.per_second",
          "dram__throughput.avg.pct_of_peak_sustained_elapsed",
          "l1tex__average_t_sectors_per_request_pipe_lsu_mem_local_op_ld.ratio",
          "l1tex__average_t_sectors_per_request_pipe_lsu_mem_global_op_st."
          "ratio",
          "l1tex__data_pipe_lsu_wavefronts_mem_shared.avg.pct_of_peak_"
          "sustained_elapsed",
          "l1tex__data_pipe_lsu_wavefronts_mem_shared_op_ld.sum.per_second",
          "l1tex__data_pipe_lsu_wavefronts_mem_shared_op_st.sum.per_second",
          "l1tex__f_tex2sm_cycles_active.avg.pct_of_peak_sustained_elapsed",
          "l1tex__lsu_writeback_active.avg.pct_of_peak_sustained_active",
          "l1tex__tex_writeback_active.avg.pct_of_peak_sustained_active",
          "l1tex__t_bytes_pipe_lsu_mem_global_op_ld.sum.per_second",
          "l1tex__t_bytes_pipe_lsu_mem_global_op_st.sum.per_second",
          "l1tex__t_bytes_pipe_lsu_mem_local_op_ld.sum.per_second",
          "l1tex__t_sector_hit_rate.pct",
          "l1tex__t_sectors_pipe_lsu_mem_local_op_st.sum.per_second",
          "l1tex__texin_sm2tex_req_cycles_active.avg.pct_of_peak_sustained_"
          "elapsed",
          "lts__t_sector_hit_rate.pct", "lts__t_sector_op_read_hit_rate.pct",
          "lts__t_sector_op_write_hit_rate.pct",
          "lts__t_sectors.avg.pct_of_peak_sustained_elapsed",
          "lts__t_sectors_aperture_sysmem_op_read.sum.per_second",
          "lts__t_sectors_aperture_sysmem_op_write.sum.per_second",
          "lts__t_sectors_op_read.sum.per_second",
          "lts__t_sectors_op_write.sum.per_second",
          "lts__t_sectors_srcunit_l1_op_atom.sum.per_second",
          "lts__t_sectors_srcunit_tex_op_read.sum.per_second",
          "sm__mio2rf_writeback_active.avg.pct_of_peak_sustained_elapsed",
          "sm__pipe_tensor_cycles_active.avg.pct_of_peak_sustained_active",
          "sm__warps_active.avg.pct_of_peak_sustained_active",
          "smsp__average_inst_executed_per_warp.ratio",
          "smsp__cycles_active.avg.pct_of_peak_sustained_elapsed",
          "smsp__inst_executed.avg.per_cycle_active",
          "smsp__inst_executed_pipe_fp16.avg.pct_of_peak_sustained_active",
          "smsp__inst_executed_pipe_fp64.avg.pct_of_peak_sustained_active",
          "smsp__inst_executed_pipe_lsu.avg.pct_of_peak_sustained_active",
          "smsp__inst_executed_pipe_tex.avg.pct_of_peak_sustained_active",
          "smsp__inst_executed_pipe_xu.avg.pct_of_peak_sustained_active",
          "smsp__inst_issued.avg.per_cycle_active",
          "smsp__issue_active.avg.pct_of_peak_sustained_active",
          "smsp__pipe_fma_cycles_active.avg.pct_of_peak_sustained_active",
          "smsp__sass_average_branch_targets_threads_uniform.pct",
          "smsp__sass_average_data_bytes_per_sector_mem_global_op_ld.pct",
          "smsp__sass_average_data_bytes_per_sector_mem_global_op_st.pct",
#if 0 // get inf when compiled with clang++ instead of nvcc, so nvidia fxxk you
          "smsp__sass_average_data_bytes_per_wavefront_mem_shared.pct",
#endif
          "smsp__sass_thread_inst_executed_ops_dadd_dmul_dfma_pred_on.avg.pct_"
          "of_peak_sustained_elapsed",
          "smsp__sass_thread_inst_executed_ops_hadd_hmul_hfma_pred_on.avg.pct_"
          "of_peak_sustained_elapsed",
          "smsp__sass_thread_inst_executed_ops_fadd_fmul_ffma_pred_on.avg.pct_"
          "of_peak_sustained_elapsed",
#if 0 // get error, so nvidia fxxk you
          "smsp__thread_inst_executed_per_inst_executed.pct"
#endif
          "smsp__thread_inst_executed_per_inst_executed.ratio",
          "smsp__warps_eligible.sum.per_cycle_active",
          "smsp__warp_issue_stalled_barrier_per_warp_active.pct",
          "smsp__warp_issue_stalled_dispatch_stall_per_warp_active.pct",
          "smsp__warp_issue_stalled_drain_per_warp_active.pct",
          "smsp__warp_issue_stalled_imc_miss_per_warp_active.pct",
          "smsp__warp_issue_stalled_lg_throttle_per_warp_active.pct",
          "smsp__warp_issue_stalled_long_scoreboard_per_warp_active.pct",
          "smsp__warp_issue_stalled_math_pipe_throttle_per_warp_active.pct",
          "smsp__warp_issue_stalled_membar_per_warp_active.pct",
          "smsp__warp_issue_stalled_mio_throttle_per_warp_active.pct",
          "smsp__warp_issue_stalled_misc_per_warp_active.pct",
          "smsp__warp_issue_stalled_no_instruction_per_warp_active.pct",
          "smsp__warp_issue_stalled_not_selected_per_warp_active.pct",
          "smsp__warp_issue_stalled_short_scoreboard_per_warp_active.pct",
          "smsp__warp_issue_stalled_tex_throttle_per_warp_active.pct",
          "smsp__warp_issue_stalled_wait_per_warp_active.pct",
          "sm__cycles_active.sum", "sm__cycles_elapsed.sum",
#endif
          "dram__throughput.avg.pct_of_peak_sustained_elapsed",
          "l1tex__t_sector_hit_rate.pct", "lts__t_sector_hit_rate.pct",
          "lts__t_sectors.avg.pct_of_peak_sustained_elapsed",
          "sm__cycles_elapsed.sum", "sm__cycles_active.sum",
          "sm__mio2rf_writeback_active.avg.pct_of_peak_sustained_elapsed",
          "smsp__inst_executed_pipe_lsu.avg.pct_of_peak_sustained_active",
          "smsp__issue_active.avg.pct_of_peak_sustained_active",
          "smsp__pipe_fma_cycles_active.avg.pct_of_peak_sustained_active",
          "smsp__warp_issue_stalled_long_scoreboard_per_warp_active.pct",
          "smsp__warp_issue_stalled_math_pipe_throttle_per_warp_active.pct",
          "smsp__warp_issue_stalled_no_instruction_per_warp_active.pct",
          "smsp__warp_issue_stalled_not_selected_per_warp_active.pct",
          "smsp__warp_issue_stalled_short_scoreboard_per_warp_active.pct",
          "smsp__warp_issue_stalled_wait_per_warp_active.pct",
          "sm__warps_active.avg.pct_of_peak_sustained_active",
          "smsp__warps_eligible.sum.per_cycle_active"
    };
    wuk::CuProfiler p(metricNames);
    p.ProfileKernels("RangeName", reset, kernel);
    std::string res = wuk::CuProfiler::res_to_json(p.MetricValues());
    if (res.back() == ']')
      res.pop_back();
    if (res.front() == '[')
      res = res.substr(1);
    ret += ", \"Metric\": " + res;
  } while (0);
#endif

  return "{" + ret + "}";
}

} // namespace test

} // namespace GoPTX