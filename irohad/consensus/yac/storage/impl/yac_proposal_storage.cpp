/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include "consensus/yac/storage/yac_proposal_storage.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      // --------| private api |--------

      auto YacProposalStorage::findStore(ProposalHash proposal_hash,
                                         BlockHash block_hash) {

        // find exist
        auto iter =
            std::find_if(block_storages_.begin(), block_storages_.end(),
                         [&proposal_hash, &block_hash](auto block_storage) {
                           auto yac_hash = block_storage.getStorageHash();
                           return yac_hash.proposal_hash == proposal_hash and
                               yac_hash.block_hash == block_hash;
                         });
        if (iter != block_storages_.end()) {
          return iter;
        }
        // insert and return new
        return block_storages_
            .emplace(block_storages_.end(),
                     YacHash(proposal_hash, block_hash), peers_in_round_);
      }

      // --------| public api |--------

      YacProposalStorage::YacProposalStorage(ProposalHash hash,
                                             uint64_t peers_in_round)
          : current_state_(nonstd::nullopt),
            hash_(std::move(hash)),
            peers_in_round_(peers_in_round) {
      }

      nonstd::optional<Answer> YacProposalStorage::insert(VoteMessage msg) {
        if (shouldInsert(msg)) {
          // insert to block store
          auto iter = findStore(msg.hash.proposal_hash, msg.hash.block_hash);
          auto block_state = iter->insert(msg);

          if (block_state.has_value() and
              block_state->commit.has_value()) {
            // supermajority on block achieved
            current_state_ = *block_state;
          } else {
            // try to find reject case
            auto reject = findRejectProof();
            if (reject.has_value()) {
              current_state_ = *reject;
            }
          }
        }
        return getState();
      };

      nonstd::optional<Answer> YacProposalStorage::insert(std::vector<
          VoteMessage> messages) {
        std::for_each(messages.begin(), messages.end(),
                      [this](auto vote) {
                        this->insert(std::move(vote));
                      });
        return getState();
      }
      ProposalHash YacProposalStorage::getProposalHash() {
        return hash_;
      }

      nonstd::optional<Answer> YacProposalStorage::getState() const {
        return current_state_;
      }

      // --------| private api |--------

      bool YacProposalStorage::shouldInsert(const VoteMessage &msg) {
        return checkProposalHash(msg.hash.proposal_hash) and
            checkPeerUniqueness(msg);
      }

      bool YacProposalStorage::checkProposalHash(ProposalHash vote_hash) {
        return vote_hash == hash_;
      }

      bool YacProposalStorage::checkPeerUniqueness(const VoteMessage &msg) {
        // todo implement method: checking based on public keys
        return true;
      }

      nonstd::optional<Answer> YacProposalStorage::findRejectProof() {
        // todo implement
        return nonstd::nullopt;
      }

    } // namespace yac
  } // namespace consensus
} // namespace iroha
