/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ConfigFiles.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace redex_properties {

struct PropertyInteraction {
  bool establishes{false};
  bool requires_{false};
  bool preserves{false};
  // "destroys" when !establishes && !preserves;
  bool is_valid() const {
    if (requires_ && establishes && !preserves) {
      return false;
    }
    return true;
  }
};

using PropertyName = std::string;

namespace names {
inline const std::string NoInitClassInstructions("NoInitClassInstructions");
// Stand-in for fixing up passes.
inline const std::string NeedsEverythingPublic("NeedsEverythingPublic");
inline const std::string HasSourceBlocks("HasSourceBlocks");
} // namespace names

bool property_is_enabled(const PropertyName& property_name,
                         const ConfigFiles& conf);

std::unordered_set<PropertyName> get_initial(const ConfigFiles& conf);
std::unordered_set<PropertyName> get_final(const ConfigFiles& conf);

using PropertyInteractions =
    std::unordered_map<PropertyName, PropertyInteraction>;

std::unordered_set<PropertyName> get_required(
    const PropertyInteractions& interactions);
std::unordered_set<PropertyName> apply(
    std::unordered_set<PropertyName> established_properties,
    const PropertyInteractions& interactions);
std::optional<std::string> verify_pass_interactions(
    const std::vector<std::pair<std::string, PropertyInteractions>>&
        pass_interactions,
    ConfigFiles& conf);

} // namespace redex_properties