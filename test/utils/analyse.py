# spack load --first py-altair py-vl-convert-python
# python3 analyse.py < results.html
import altair as alt  # altair>=5.0.1

from json import loads
from statistics import mean, geometric_mean

import re
import sys


def chart_saver(chart, name):
    try:
        from vl_convert import vegalite_to_pdf

        open("figure." + name + ".pdf", "wb").write(
            vegalite_to_pdf(vl_spec=chart.to_json())
        )
    except ImportError:
        chart.save("figure." + name + ".html")


def wuk_theme():
    alt.themes.register(
        "wuk",
        lambda: {
            "config": {
                "axis": {
                    "ticks": None,
                    "title": None,
                    "titleFontWeight": "normal",
                },
                "axisX": {"labelAngle": -45},
                "background": "rgba(0,0,0,0)",
                # "font": "serif",
                "header": {"title": None},
                "legend": {"title": None},
                "mark": {
                    # "stroke": "#79706E",
                },
                "range": {
                    "category": [
                        "#439894",  # dark green
                        "#fcbfd2",  # pink
                        "#79706e",  # dark grey
                        "#83bcb6",  # green
                        "#d67195",  # dark pink
                        "#bab0ac",  # grey
                    ]  # { "scheme": "tableau20",  # https://vega.github.io/vega/docs/schemes/}
                },
            }
        },
    )
    alt.themes.enable("wuk")


def wash_data(data):
    # https://docs.nvidia.com/cupti/main/main.html#metrics-mapping-table
    metrics = {
        "sm__warps_active.avg.pct_of_peak_sustained_active": "AOC",
        "smsp__sass_average_branch_targets_threads_uniform.pct": "BE",
        "smsp__sass_average_data_bytes_per_sector_mem_global_op_ld.pct": "GLE",
        "smsp__sass_average_data_bytes_per_sector_mem_global_op_st.pct": "GSE",
        # "smsp__inst_executed_pipe_fp16.avg.pct_of_peak_sustained_active": "half_precision_fu_utilization",
        # "smsp__sass_average_data_bytes_per_wavefront_mem_shared.pct": "shared_efficiency",
        "smsp__cycles_active.avg.pct_of_peak_sustained_elapsed": "SME",
        # "sm__pipe_tensor_cycles_active.avg.pct_of_peak_sustained_active": "tensor_precision_fu_utilization",
        "dram__throughput.avg.pct_of_peak_sustained_elapsed": "DU",
        # "smsp__sass_thread_inst_executed_ops_fadd_fmul_ffma_pred_on.avg.pct_of_peak_sustained_elapsed": "flop_sp_efficiency",
        "smsp__issue_active.avg.pct_of_peak_sustained_active": "ISU",
        "l1tex__t_sector_hit_rate.pct": "THR",
        # "l1tex__texin_sm2tex_req_cycles_active.avg.pct_of_peak_sustained_elapsed": "STU",
        # "l1tex__f_tex2sm_cycles_active.avg.pct_of_peak_sustained_elapsed": "TSTU",
        "sm__mio2rf_writeback_active.avg.pct_of_peak_sustained_elapsed": "TSU",
        "lts__t_sector_hit_rate.pct": "L2HR",
        "lts__t_sectors.avg.pct_of_peak_sustained_elapsed": "L2U",
        "smsp__inst_executed_pipe_lsu.avg.pct_of_peak_sustained_active": "LSU",
        "l1tex__data_pipe_lsu_wavefronts_mem_shared.avg.pct_of_peak_sustained_elapsed": "SU",
        "smsp__pipe_fma_cycles_active.avg.pct_of_peak_sustained_active": "SPU",
        # "smsp__inst_executed_pipe_xu.avg.pct_of_peak_sustained_active": "special_fu_utilization",
        "smsp__thread_inst_executed_per_inst_executed.ratio": "WEE",
        "smsp__inst_executed.avg.per_cycle_active": "IPC",
        # "smsp__inst_issued.avg.per_cycle_active": "issued_ipc",
        "smsp__warps_eligible.sum.per_cycle_active": "EWPC",
        "sm__cycles_elapsed.sum": "CE",
        "sm__cycles_active.sum": "CA",
    }

    ret = []
    for it in data:
        strategy = it["Strategy"]
        if strategy.startswith("GoPTX_strategy_"):
            strategy = strategy[len("GoPTX_strategy_") :]
        bf = 0
        bs = 0
        if strategy.startswith("GoPTX_concat"):
            matchobj = re.findall(r"\d+", strategy)
            if len(matchobj) == 2:
                bf = matchobj[0]
                bs = matchobj[1]
            strategy = "GoPTX_concat"
        elif strategy.startswith("GoPTX_even"):
            matchobj = re.findall(r"\d+", strategy)
            if len(matchobj) == 2:
                bf = matchobj[0]
                bs = matchobj[1]
            strategy = "GoPTX_even"
        elif strategy.startswith("GoPTX_latency"):
            matchobj = re.findall(r"\d+", strategy)
            if len(matchobj) == 2:
                bf = matchobj[0]
                bs = matchobj[1]
            strategy = "GoPTX_latency"
        first = it["First"]
        second = it["Second"]
        tmp = {
            "Strategy": strategy,
            "First": first,
            "Second": second,
            "BF": bf,
            "BS": bs,
            "Time": it["min(ms)"],
        }
        if "Metric" in it:
            for k, v in it["Metric"]["Metrics"].items():
                if k in metrics:
                    k = metrics[k]
                    tmp[k] = v
                elif k.startswith("smsp__warp_issue_stalled_"):
                    k = k[
                        len("smsp__warp_issue_stalled_") : k.index(
                            "_per_warp_active.pct"
                        )
                    ]
                    tmp[k] = v
        ret.append(tmp)

    for dat in ret:
        if dat["Strategy"].find("cuStream") >= 0:
            dat["Strategy"] = "  Baseline"
        elif dat["Strategy"].find("hfuse") >= 0:
            dat["Strategy"] = " HFuse"
        elif dat["Strategy"].find("vfuse") >= 0:
            dat["Strategy"] = "  VFuse"
        elif dat["Strategy"].find("first") >= 0:
            dat["Strategy"] = "first"
        elif dat["Strategy"].find("second") >= 0:
            dat["Strategy"] = "second"
        else:
            dat["Strategy"] = "GoPTX"
    return ret


def draw_motivated_EWPC(data):
    chart = [
        {"EWPC": dat["EWPC"], "First": dat["First"]}
        for dat in data
        if dat["Second"].find("WMMA") >= 0 and dat["Strategy"].find("first") >= 0
    ]
    chart = (
        alt.Chart(alt.InlineData(chart))
        .encode(
            y=alt.Y("First:N"),
            x=alt.X("EWPC:Q", title="eligible warps per cycle"),
        )
        .mark_bar(fill="#bab0ac")
        .properties(height=alt.Step(12), width=160)
        .configure_axisX(labelAngle=0)
    )
    chart_saver(chart, "motivated_EWPC")


def draw_motivated_stall_reasons(data):
    sortReasons = [
        "long_scoreboard",
        # "short_scoreboard",
        "wait",
        "not_selected",
        "math_pipe_throttle",
        "lg_throttle",
        "mio_throttle",
        "drain",
        "dispatch_stall",
        "no_instruction",
        # "barrier",
        # "imc_miss",
        "Others",
    ]

    chart = []
    for dat in data:
        if dat["Second"].find("WMMA") >= 0 and dat["Strategy"].find("first") >= 0:
            for k, v in dat.items():
                if k in sortReasons:
                    chart.append(
                        {
                            "First": dat["First"],
                            "Strategy": dat["Strategy"],
                            "Reason": k,
                            "Percent": v,
                            "Order": sortReasons.index(k),
                        }
                    )
    chart = (
        alt.Chart(data=alt.InlineData(chart))
        .encode(
            y=alt.Y("First:N", axis=None),
            x=alt.X(
                "sum(Percent):Q",
                scale=alt.Scale(domain=(0, 100)),
                title="Stall Percent (%)",
            ),
            order=alt.Order("Order:Q"),
            color=alt.Color("Reason:N", sort=sortReasons),
        )
        .mark_bar()
        .properties(height=alt.Step(12), width=100)
        .configure_axisX(labelAngle=0)
    )
    chart_saver(chart, "motivated_stall_reasons")


def draw_table(data):
    BaselineMetric = {}
    for dat in data:
        if dat["Strategy"].find("Baseline") >= 0:
            BaselineMetric[(dat["First"], dat["Second"])] = dat
    mapper = {}
    for metric in ["Time"]:
        for dat in data:
            s = (
                1
                - (dat[metric] + 1e-9)
                / (BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9)
                + 1
            )
            mapper[(dat["First"], dat["Second"], dat["Strategy"], metric)] = s
    for metric in ["EWPC"]:
        for dat in data:
            s = (dat[metric] + 1e-9) / (
                BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9
            )
            mapper[(dat["First"], dat["Second"], dat["Strategy"], metric)] = s

    for first in ["GELU", "HARRIS", "LUD", "MICND", "SORT", "STMS", "WMMA"]:
        print("\\multirow{3}{*}{" + first + "}", end="")
        for strategy in ["  VFuse", " HFuse", "GoPTX"]:
            print(" & " + strategy, end="")
            for metric in ["Time", "EWPC"]:
                for second in [
                    "GELU",
                    "HARRIS",
                    "LUD",
                    "MICND",
                    "SORT",
                    "STMS",
                    "WMMA",
                ]:
                    print(
                        " & %.2f"
                        % mapper[
                            (
                                first,
                                second,
                                strategy,
                                metric,
                            )
                        ],
                        end="",
                    )
            print("\\\\")
        print("\\hline")


def draw_Time(data):
    BaselineMetric = {}
    for dat in data:
        if dat["Strategy"].find("Baseline") >= 0:
            BaselineMetric[(dat["First"], dat["Second"])] = dat
    for metric in ["Time"]:
        chart = []
        for dat in data:
            s = 1 - (dat[metric] + 1e-9) / (
                BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9
            )
            chart.append(
                {
                    "Strategy": dat["Strategy"],
                    "SpeedUp": s,
                    "First": dat["First"],
                }
            )
        mapper = {}
        for it in chart:
            k = (it["First"], it["Strategy"])
            if k not in mapper:
                mapper[k] = []
            mapper[k].append(1 - it["SpeedUp"])
            if ("geomean", it["Strategy"]) not in mapper:
                mapper[("geomean", it["Strategy"])] = []
            mapper[("geomean", it["Strategy"])].append(1 - it["SpeedUp"])
        chart_bar = [
            {"First": k[0], "SpeedUp": 1 - geometric_mean(v), "Strategy": k[1]}
            for k, v in mapper.items()
        ]
        print([dat for dat in chart_bar if dat["First"].find("geomean") >= 0])
        for dat in chart_bar:
            dat["SpeedUp"] += 1
        chart_bar = (
            alt.Chart(alt.InlineData(chart_bar))
            .encode(
                x=alt.X("First:N"),
                y=alt.Y(
                    "SpeedUp:Q",
                    title="Relative Speedup",
                    scale=alt.Scale(domain=(0.0, 1.4)),
                ),
                color=alt.Color("Strategy:N"),
                xOffset=alt.XOffset("Strategy:N"),
            )
            .mark_bar()
        )
        chart = chart_bar  # + chart
        chart = (
            chart.properties(width=alt.Step(16))
            .configure_legend(
                padding=10,
                gradientDirection="horizontal",
                offset=-405,
                labelFontSize=20,
            )
            .configure_axis(labelFontSize=20, titleFontSize=20)
            .configure_axisX(labelAngle=0)
        )
        chart_saver(chart, metric)


def draw_EWPC(data):
    BaselineMetric = {}
    for dat in data:
        if dat["Strategy"].find("Baseline") >= 0:
            BaselineMetric[(dat["First"], dat["Second"])] = dat
    chart = []
    for metric in ["EWPC"]:
        for dat in data:
            chart.append(
                {
                    "Strategy": dat["Strategy"],
                    "SpeedUp": (dat[metric] + 1e-9)
                    / (BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9)
                    - 1,
                    metric: dat[metric],
                    "First": dat["First"],
                }
            )
            chart.append(
                {
                    "Strategy": dat["Strategy"],
                    "SpeedUp": (dat[metric] + 1e-9)
                    / (BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9)
                    - 1,
                    metric: dat[metric],
                    "First": "mean",
                }
            )
        mapper = {}
        mapper1 = {}
        for it in chart:
            k = (it["First"], it["Strategy"])
            if k not in mapper:
                mapper[k] = []
                mapper1[k] = []
            mapper[k].append(1 + it["SpeedUp"])
            mapper1[k].append(it[metric])
        chart = [
            {
                "First": k[0],
                metric: mean(v),
                "Strategy": k[1],
            }
            for k, v in mapper1.items()
        ]
        chart = (
            (
                alt.Chart(alt.InlineData(chart))
                .encode(
                    x=alt.X("First:N"),
                    y=alt.Y(
                        metric + ":Q",
                        title="eligible warps per cycle",
                        scale=alt.Scale(domain=(0, 1400)),
                    ),
                    color=alt.Color("Strategy:N"),
                    xOffset=alt.XOffset("Strategy:N"),
                )
                .mark_bar()
            )
            .properties(width=alt.Step(15))
            .configure_legend(
                padding=3,
                gradientDirection="horizontal",
                offset=-360,
                labelFontSize=18,
            )
            .configure_axis(labelFontSize=20, titleFontSize=20)
            .configure_axisX(labelAngle=0)
        )
        chart_saver(chart, metric)


def draw_utilization(data):
    BaselineMetric = {}
    for dat in data:
        if dat["Strategy"].find("Baseline") >= 0:
            BaselineMetric[(dat["First"], dat["Second"])] = dat
    metricSort = [
        "AOC",
        "DU",
        "L2U",
        "L2HR",
        "ISU",
        "LSU",
        "SPU",
        "TSU",
        "THR",
    ]
    chart = []
    for metric in metricSort:
        mapper = {}
        for dat in data:
            if dat["Strategy"] not in mapper:
                mapper[dat["Strategy"]] = []
            mapper[dat["Strategy"]].append(dat[metric])
        for k, v in mapper.items():
            chart.append(
                {
                    "Strategy": k,
                    "Utilaization": mean(v),
                    "Metric": metric,
                }
            )
    chart = (
        alt.Chart(data=alt.InlineData(chart))
        .encode(
            y=alt.Y(
                "Utilaization:Q",
                title="Percent",
                scale=alt.Scale(domain=(0, 100)),
            ),
            x=alt.X("Metric:N", sort=metricSort),
            color=alt.Color("Strategy:N"),
            xOffset=alt.XOffset("Strategy:N"),
        )
        .mark_bar()
        .properties(width=alt.Step(16))
        .configure_legend(
            padding=10,
            gradientDirection="horizontal",
            offset=-480,
            labelFontSize=24,
        )
        .configure_axis(labelFontSize=24, titleFontSize=24)
        .configure_axisX(labelAngle=0)
    )
    chart_saver(chart, "utilization")


def draw_warp_stall(data):
    sortReasons = [
        "long_scoreboard",
        # "short_scoreboard",
        "wait",
        "not_selected",
        "math_pipe_throttle",
        "lg_throttle",
        "mio_throttle",
        "drain",
        "dispatch_stall",
        "no_instruction",
        # "barrier",
        # "imc_miss",
        "Others",
    ]

    chart = []
    for dat in data:
        if (dat["First"].find("WMMA") >= 0 and dat["Second"].find("HARRIS") >= 0) or (
            dat["First"].find("WMMA") >= 0 and dat["Second"].find("GELU") >= 0
        ):
            percent = 100
            for k, v in dat.items():
                if k in sortReasons:
                    chart.append(
                        {
                            "Strategy": dat["Strategy"],
                            "Reason": k,
                            "Cycles": 0.01 * v * (dat["CE"] - dat["CA"]),
                            "Order": sortReasons.index(k),
                            "Second": dat["First"] + "+" + dat["Second"],
                        }
                    )
                    percent -= v
            chart.append(
                {
                    "Strategy": dat["Strategy"],
                    "Reason": "Others",
                    "Cycles": 0.01 * percent * (dat["CE"] - dat["CA"]),
                    "Order": 99,
                    "Second": "WMMA+" + dat["Second"],
                }
            )
    for dat in chart:
        dat["Cycles"] *= 1
    chart = (
        alt.Chart(data=alt.InlineData(chart))
        .encode(
            y=alt.Y("Strategy:N"),
            x=alt.X("sum(Cycles):Q", title="Number of Cycles"),
            order=alt.Order("Order:Q"),
            color=alt.Color("Reason:N", sort=sortReasons),
            column=alt.Column(
                "Second:N", header=alt.Header(labelOrient="top", labelFontSize=15)
            ),
        )
        .mark_bar()
        .properties(height=alt.Step(24), width=300)
        .configure_legend(
            gradientDirection="horizontal",
            orient="top-left",
            offset=-60,
            labelFontSize=15,
        )
        .configure_axis(labelFontSize=16, titleFontSize=16)
        .configure_axisX(labelAngle=0)
    )
    chart_saver(chart, "smsp__warp_issue_stalled")


def draw_sensitivity_threshold():
    sortThresholds = [" 256", " 512", " 768", "1024", "  noSlicing", "   adaptive"]
    chart_bar = []
    for metric in ["Time"]:
        for threshold in sortThresholds:
            f = "results." + threshold.strip() + ".html"
            f = open(f)
            data = f.read()
            f.close()
            data = loads(data[data.index("[") : data.rindex("]") + 1])
            data = wash_data(data)
            BaselineMetric = {}
            for dat in data:
                if dat["Strategy"].find("Baseline") >= 0:
                    BaselineMetric[(dat["First"], dat["Second"])] = dat
            chart = []
            for dat in data:
                s = 1 - (dat[metric] + 1e-9) / (
                    BaselineMetric[(dat["First"], dat["Second"])][metric] + 1e-9
                )
                chart.append(
                    {
                        "Strategy": dat["Strategy"],
                        "SpeedUp": s,
                        "First": dat["First"],
                    }
                )
            mapper = {}
            for it in chart:
                k = (it["First"], it["Strategy"])
                if k not in mapper:
                    mapper[k] = []
                mapper[k].append(1 - it["SpeedUp"])
                if ("geomean", it["Strategy"]) not in mapper:
                    mapper[("geomean", it["Strategy"])] = []
                mapper[("geomean", it["Strategy"])].append(1 - it["SpeedUp"])
            chart_bar += [
                {
                    "First": k[0],
                    "SpeedUp": 1 - geometric_mean(v),
                    "Threshold": threshold,
                }
                for k, v in mapper.items()
                if k[1].find("GoPTX") >= 0
            ]
    for dat in chart_bar:
        dat["SpeedUp"] += 1
    chart_point = chart_bar
    chart_bar = (
        alt.Chart(alt.InlineData(chart_bar))
        .encode(
            x=alt.X("First:N"),
            y=alt.Y(
                "SpeedUp:Q",
                title="Relative SppedUp",
                scale=alt.Scale(domain=(1.0, 1.2)),
            ),
            color=alt.Color("Threshold:N"),
            strokeDash=alt.StrokeDash("Threshold:N"),
        )
        .mark_line()
    )
    chart_point = (
        alt.Chart(alt.InlineData(chart_point))
        .encode(
            x=alt.X("First:N"),
            y=alt.Y(
                "SpeedUp:Q",
                title="Relative SppedUp",
                scale=alt.Scale(domain=(1.0, 1.2)),
            ),
            color=alt.Color("Threshold:N"),
            strokeDash=alt.StrokeDash("Threshold:N"),
        )
        .mark_point(size=64)
    )
    chart = chart_bar + chart_point
    chart = (
        chart.properties(width=alt.Step(56), height=180)
        .configure_legend(
            orient="top",
            offset=-28,
            padding=8,
            labelFontSize=14,
        )
        .configure_axisX(labelAngle=0)
        .configure_axis(labelFontSize=14, titleFontSize=14)
    )
    chart_saver(chart, "sensitivity_threshold")


def main(*argv):
    wuk_theme()

    data = sys.stdin.read()
    data = loads(data[data.index("[") : data.rindex("]") + 1])
    data = wash_data(data)

    draw_motivated_EWPC(data)

    draw_motivated_stall_reasons(data)

    data = [
        dat
        for dat in data
        if dat["Strategy"].find("Baseline") >= 0
        or dat["Strategy"].find("VFuse") >= 0
        or dat["Strategy"].find("HFuse") >= 0
        or dat["Strategy"].find("GoPTX") >= 0
    ]

    draw_table(data)

    draw_Time(data)

    draw_EWPC(data)

    draw_utilization(data)

    draw_warp_stall(data)

    draw_sensitivity_threshold()

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv))
