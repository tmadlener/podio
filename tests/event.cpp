#include "podio/CollisionPolicies.h"
#include "podio/Frame.h"
#include "podio/ROOTRawData.h"
#include "podio/UnpackingPolicies.h"

#include "catch2/catch_test_macros.hpp"

#include "datamodel/ExampleHitCollection.h"

struct LazyUnpackingDefaults : podio::FrameDefaultPolicies {
  using UnpackingPolicy = podio::LazyUnpacking;
};

TEST_CASE("Frame", "[event][basics]") {
  auto d = std::make_unique<podio::ROOTRawData>();
  auto event = podio::Frame(std::move(d), LazyUnpackingDefaults{});

  auto event2 = podio::Frame(LazyUnpackingDefaults{});

  auto d2 = std::make_unique<podio::ROOTRawData>();
  auto event3 = podio::Frame(std::move(d2));

  auto event4 = podio::Frame();

  REQUIRE(true);
}

TEST_CASE("Frame::put", "[event][basics]") {
  auto event = podio::Frame(LazyUnpackingDefaults{});
  auto hitColl = ExampleHitCollection();
  for (size_t i = 0; i < 10; ++i) {
    auto hit = hitColl.create();
    hit.energy(i);
  }

  REQUIRE_FALSE(event.contains("hits"));
  auto& hits = event.put(std::move(hitColl), "hits");
  REQUIRE(event.contains("hits"));
  for (size_t i = 0; i < 10; ++i) {
    REQUIRE(hits[i].energy() == i);
  }

  auto& getHits = event.get<ExampleHitCollection>("hits");
  for (size_t i = 0; i < 10; ++i) {
    REQUIRE(getHits[i].energy() == i);
  }
}

TEST_CASE("Frame::put collision", "[event]") {
  auto hitColl = ExampleHitCollection();
  for (size_t i = 0; i < 10; ++i) {
    auto hit = hitColl.create();
    hit.energy(i);
  }

  SECTION("ThrowOnCollision") {
    auto event = podio::Frame();
    event.put(std::move(hitColl), "hits");
    REQUIRE_THROWS_AS(event.put(ExampleHitCollection(), "hits"), std::runtime_error);
  }

  SECTION("KeepOriginal") {
    struct IgnoreCollisionPolicies : podio::FrameDefaultPolicies {
      using CollisionPolicy = podio::KeepOriginal;
    };

    auto event = podio::Frame(IgnoreCollisionPolicies{});
    event.put(std::move(hitColl), "hits");
    const auto& origColl = event.put(ExampleHitCollection(), "hits");
    // In this case the new collection gets discarded and we get back the
    // orginal collection
    REQUIRE(origColl.size() == 10);
    for (size_t i = 0; i < 10; ++i) {
      REQUIRE(origColl[i].energy() == i);
    }
  }

  SECTION("ReplaceCollection") {
    struct ReplaceOnCollisionPolicies : podio::FrameDefaultPolicies {
      using CollisionPolicy = podio::ReplaceCollection;
    };

    auto event = podio::Frame(ReplaceOnCollisionPolicies{});
    event.put(std::move(hitColl), "hits");

    auto moreHits = ExampleHitCollection();
    for (size_t i = 0; i < 5; ++i) {
      auto hit = moreHits.create();
      hit.energy(i * i);
    }

    const auto& placedHits = event.put(std::move(moreHits), "hits");
    REQUIRE(placedHits.size() == 5);
    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(placedHits[i].energy() == i * i);
    }
  }
}

TEST_CASE("Metadata", "[event]") {
  auto event = podio::Frame();
  // event.putMetaData("someVal", 1);
  // event.putMetaData("someOtherVal", {3.14f, 42.0f});

  // auto val = event.getMetaData<int>("someVal");
}
