/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MergerType.h"

#include "Trace.h"
#include <sstream>
#include <string>

namespace class_merging {

/**
 * Extract a minimal but identifiable name tag from the given root type.
 * E.g., "Lcom/facebook/analytics/structuredlogger/base/TypedEventBase;" ->
 * "EBase"
 */
std::string get_type_name_tag(const DexType* root_type) {
  auto root_type_name = type::get_simple_name(root_type);
  std::ostringstream root_name_tag;
  std::string::reverse_iterator rit = root_type_name.rbegin();
  // Scan the string from back to front.
  // Take out the last word in the simple type name starting with a cap letter.
  // E.g., "Lcom/facebook/TypedEventBase;" -> "esaB".
  for (; rit != root_type_name.rend(); ++rit) {
    auto c = *rit;
    if (isupper(c)) {
      root_name_tag << c;
      break;
    } else {
      root_name_tag << c;
    }
  }
  // Advance if it's not the end.
  if (rit != root_type_name.rend()) {
    ++rit;
  }
  // Keep scanning backwards. Find the first cap letter of the second to last
  // word if any.
  // E.g., "Lcom/facebook/TypedEventBase;" -> "esaBE".
  for (; rit != root_type_name.rend(); ++rit) {
    auto c = *rit;
    if (isupper(c)) {
      root_name_tag << c;
      break;
    }
  }
  auto root_name_tag_str = root_name_tag.str();
  // Apparantly, since we were traversing in reverse, we need to reverse the
  // name tag string.
  // E.g., "esaBE" -> "EBase".
  std::reverse(root_name_tag_str.begin(), root_name_tag_str.end());
  TRACE(CLMG, 7, "  root_name_tag %s", root_name_tag_str.c_str());
  return root_name_tag_str;
}

std::string MergerType::Shape::build_type_name(
    const std::string& prefix,
    const DexType* root_type,
    const TypeSet& intf_set,
    const boost::optional<size_t>& dex_id,
    size_t count,
    const boost::optional<InterdexSubgroupIdx>& interdex_subgroup_idx,
    const InterdexSubgroupIdx subgroup_idx) const {
  auto parent = root_type;
  if (root_type == type::java_lang_Object() && intf_set.size() == 1) {
    parent = *intf_set.begin();
  }
  auto root_name_tag = get_type_name_tag(parent);
  std::ostringstream ss;
  ss << "L" << prefix << root_name_tag << "Shape";
  ss << count << "S" << string_fields << reference_fields << bool_fields
     << int_fields << long_fields << double_fields << float_fields;

  if (dex_id && *dex_id > 0) {
    ss << "_" << *dex_id;
  }

  if (interdex_subgroup_idx != boost::none) {
    ss << "_I" << interdex_subgroup_idx.get();
  }

  if (subgroup_idx != 0) {
    ss << "_" << subgroup_idx;
  }
  ss << ";";
  return ss.str();
}

std::string MergerType::Shape::to_string() const {
  std::ostringstream ss;
  ss << "(" << string_fields << "," << reference_fields << "," << bool_fields
     << "," << int_fields << "," << long_fields << "," << double_fields << ","
     << float_fields << ")";
  return ss.str();
}

MergerType::Shape::Shape(const std::vector<DexField*>& fields) {
  for (const auto& field : fields) {
    const auto field_type = field->get_type();
    if (field_type == type::java_lang_String()) {
      string_fields++;
      continue;
    }
    switch (type::type_shorty(field_type)) {
    case 'L':
    case '[':
      reference_fields++;
      break;
    case 'J':
      long_fields++;
      break;
    case 'D':
      double_fields++;
      break;
    case 'F':
      float_fields++;
      break;
    case 'Z':
      bool_fields++;
      break;
    case 'B':
    case 'S':
    case 'C':
    case 'I':
      int_fields++;
      break;
    default:
      not_reached();
    }
  }
}

} // namespace class_merging
