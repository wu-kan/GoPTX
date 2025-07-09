#include "GoPTX/GoPTX.h"
#include <GoPTX/GoPTX.hh>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <type_traits>

template <typename Strategy = GoPTX::StrategyMerge<>>
static GoPTX::EntryOrFunc
merge_entry(GoPTX::EntryOrFunc entry_first, GoPTX::EntryOrFunc entry_second,
            const std::string &name_merged, const GoPTX_config &config) {
  assert(entry_first.k_entry.type == GoPTX::Terminal::get_type_T_WORD() &&
         !entry_first.k_entry.text.empty());
  assert(entry_second.k_entry.type == GoPTX::Terminal::get_type_T_WORD() &&
         !entry_second.k_entry.text.empty());
  entry_first.add_suffix("f");
  GoPTX::ControlFlowGraph cfg_first(entry_first.statements);
  entry_second.add_suffix("s");

  GoPTX::ControlFlowGraph cfg_second(entry_second.statements);
  if (std::is_same<Strategy,
                   GoPTX::StrategyMerge<GoPTX::StrategyVFuse>>::value ||
      std::is_same<Strategy,
                   GoPTX::StrategyMerge<GoPTX::StrategyHFuse>>::value) {

  } else {
    cfg_first.replace_aligned();
    cfg_second.replace_aligned();
    cfg_first.split_basic_block_by_latency();
    cfg_second.split_basic_block_by_latency();
  }
  Strategy strategy;
  GoPTX::ControlFlowGraph cfg_merged = strategy(cfg_first, cfg_second, config);
  entry_first.statements = cfg_merged.to_statements("__GoPTX");
  entry_first.param_list.insert(entry_first.param_list.end(),
                                entry_second.param_list.begin(),
                                entry_second.param_list.end());
  entry_first.name.text = name_merged;
#if 0
  if (std::is_same<Strategy, GoPTX::StrategyMerge<
                                 GoPTX::BlockStrategyConcat>>::value) {
    std::fprintf(stderr, "## First %s\n\n```mermaid\n%s\n```\n\n",
                 name_merged.c_str(), cfg_first.to_mermaid().c_str());
    std::fprintf(stderr, "## Second %s\n\n```mermaid\n%s\n```\n\n",
                 name_merged.c_str(), cfg_second.to_mermaid().c_str());
    std::fprintf(stderr, "## Merged %s\n\n```mermaid\n%s\n```\n\n",
                 name_merged.c_str(), cfg_merged.to_mermaid().c_str());
    std::fprintf(stderr, "## Merged PTX\n\n```ptx\n%s\n```\n\n",
                 entry_first.to_string().c_str());
    std::exit(-1);
  }
#endif
  return entry_first;
}

extern "C" {

void GoPTX_free(void *ptr) { std::free(ptr); }

void GoPTX_strdup(char **ptr, const char *s) {
  auto len = std::strlen(s);
  char *ret = (char *)std::malloc((len + 1) * sizeof(char));
  std::memcpy(ret, s, (len + 1) * sizeof(char));
  *ptr = ret;
}

void GoPTX_fread(char **ptr, const char *file_name) {
  std::FILE *pfile = std::fopen(file_name, "r");
  if (pfile == nullptr) {
    *ptr = nullptr;
    return;
  }
  std::fseek(pfile, 0, SEEK_END);
  auto len = std::ftell(pfile);
  if (!len) {
    *ptr = nullptr;
    return;
  }
  char *data = (char *)std::malloc((len + 1) * sizeof(char));
  std::fseek(pfile, 0, SEEK_SET);
  len = std::fread(data, sizeof(char), len, pfile);
  data[len] = (char)(0);
  std::fclose(pfile);
  *ptr = data;
}

void GoPTX_link(char **ptr, const char *ptx_first, const char *ptx_second,
                const char *suffix_first, const char *suffix_second) {
  auto prog_first = GoPTX::parse(ptx_first);
  auto prog_second = GoPTX::parse(ptx_second);
  auto ret = prog_first.concat(prog_second, suffix_first, suffix_second);
  assert(ret == 0);
  auto s = prog_first.to_string();
  GoPTX_strdup(ptr, s.c_str());
}

void GoPTX_merge(char **ptr, const char *ptx, const char *regex_merged,
                 const char *regex_first, const char *regex_second,
                 GoPTX_config config) {
  auto prog = GoPTX::parse(ptx);
  std::vector<size_t> indexs_first, indexs_second;
  do {
    std::regex re_first(regex_first), re_second(regex_second);
    for (size_t i = 0; i < prog.entry_or_func_list.size(); ++i) {
      if (std::regex_search(prog.entry_or_func_list[i].name.text, re_first)) {
        indexs_first.push_back(i);
      }
      if (std::regex_search(prog.entry_or_func_list[i].name.text, re_second))
        indexs_second.push_back(i);
    }
  } while (0);
  for (auto index_first : indexs_first) {
    for (auto index_second : indexs_second) {
      GoPTX::EntryOrFunc ret;
      std::string name_merged = prog.entry_or_func_list[index_first].name.text +
                                "&" +
                                prog.entry_or_func_list[index_second].name.text;
      std::regex e("([[:w:]]+)&([[:w:]]+)");
      name_merged = std::regex_replace(name_merged, e, regex_merged);
      auto identifiers = prog.get_identifiers();
      for (const auto &identifier : identifiers) {
        assert(identifier != name_merged);
      }
      switch (config.strategy) {
      case GoPTX_strategy_GoPTX_latency:
        ret = merge_entry<GoPTX::StrategyMerge<GoPTX::BlockStrategyLatency>>(
            prog.entry_or_func_list[index_first],
            prog.entry_or_func_list[index_second], name_merged, config);
        break;
      case GoPTX_strategy_GoPTX_concat:
        ret = merge_entry<GoPTX::StrategyMerge<GoPTX::BlockStrategyConcat>>(
            prog.entry_or_func_list[index_first],
            prog.entry_or_func_list[index_second], name_merged, config);
        break;
      case GoPTX_strategy_hfuse:
        ret = merge_entry<GoPTX::StrategyHFuse>(
            prog.entry_or_func_list[index_first],
            prog.entry_or_func_list[index_second], name_merged, config);
        break;
      case GoPTX_strategy_vfuse:
        ret = merge_entry<GoPTX::StrategyVFuse>(
            prog.entry_or_func_list[index_first],
            prog.entry_or_func_list[index_second], name_merged, config);
        break;
      default:
        ret = merge_entry<>(prog.entry_or_func_list[index_first],
                            prog.entry_or_func_list[index_second], name_merged,
                            config);
      }
      prog.entry_or_func_list.push_back(ret);
    }
  }
  auto s = prog.to_string();
  GoPTX_strdup(ptr, s.c_str());
}
}