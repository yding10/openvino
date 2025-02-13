// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/provenance.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ngraph/builder/norm.hpp"
#include "ngraph/graph_util.hpp"
#include "ngraph/ngraph.hpp"
#include "ngraph/pass/manager.hpp"
#include "util/provenance_enabler.hpp"

NGRAPH_SUPPRESS_DEPRECATED_START
using namespace std;
using namespace ngraph;
using ::testing::Return;

using ProvSet = std::unordered_set<std::string>;

TEST(provenance, provenance) {
    test::ProvenanceEnabler provenance_enabler;

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    // Replacement:
    //
    //       A{tag_a} B{tag_b}
    //              | |
    //         C := D{}
    //
    // After:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        D{tag_c}
    //
    // Comment:
    //   * D is the replacement root, and its insertion kills C. We should not, however, consider
    //     A and B to be killed, because they are not post-dominated by D until after C is cut out
    //     of the graph.
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto new_c = make_shared<op::v1::Subtract>(a, b);
        replace_node(c, new_c);

        EXPECT_EQ(new_c->get_provenance_tags(), ProvSet{"tag_c"});
    }

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    // Replacement:
    //
    //
    //
    //     A{tag_a}  B{tag_b}
    //        |      |
    //   C -> D{tag_d}
    //
    // After:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        D{tag_c,tag_d}
    //
    // Comment:
    //   * D is the replacement root, and its insertion kills C. We should not, however, consider
    //     A and B to be killed, because they are not post-dominated by D until after C is cut out
    //     of the graph.
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto d = make_shared<op::v1::Subtract>(a, b);
        d->add_provenance_tag("tag_d");
        replace_node(c, d);

        EXPECT_EQ(d->get_provenance_tags(), (ProvSet{"tag_c", "tag_d"}));
    }

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    // Replacement:
    //
    //   C -> D{tag_d}
    //
    // After:
    //
    //   D{tag_a,tag_b,tag_c,tag_d}
    //
    // Comment:
    //   * D is the replacement root, and its insertion kills A, B, and C.
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto d = make_zero(element::i32, Shape{2, 3, 4});
        d->add_provenance_tag("tag_d");
        replace_node(c, d);

        EXPECT_EQ(d->get_provenance_tags(), (ProvSet{"tag_a", "tag_b", "tag_c", "tag_d"}));
    }

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    // Replacement:
    //
    //   C -> D{}
    //
    // After:
    //
    //   D{tag_a,tag_b,tag_c}
    //
    // Comment:
    //   * D is the replacement root, and its insertion kills A, B, and C.
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto d = make_zero(element::i32, Shape{2, 3, 4});
        replace_node(c, d);

        EXPECT_EQ(d->get_provenance_tags(), (ProvSet{"tag_a", "tag_b", "tag_c"}));
    }

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    //
    // Replacement:
    //
    //   A{tag_a}  B{tag_b}
    //         |     |
    //        E{}    |
    //         |     |
    //    C -> D{tag_d}
    //
    //
    // After:
    //
    //   A{tag_a}          B{tag_b}
    //         |             |
    //      E{tag_c}         |
    //           |           |
    //          D{tag_c, tag_d}
    //
    // Comment:
    //   * D is the replacement root replacing C and creating a new argument node E
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto e = make_shared<op::v1::Subtract>(a, x);
        auto d = make_shared<op::v1::Subtract>(e, b);
        d->add_provenance_tag("tag_d");

        replace_node(c, d);

        EXPECT_EQ(d->get_provenance_tags(), (ProvSet{"tag_c", "tag_d"}));
        EXPECT_EQ(e->get_provenance_tags(), (ProvSet{"tag_c"}));
    }

    //
    // Before:
    //
    //   A{tag_a}  B{tag_b}
    //         |   |
    //        C{tag_c}
    //
    //
    // Replacement:
    //
    //   A{tag_a}  B{tag_b}
    //         |      |
    //       E{tag_e} |
    //           |    |
    //     C -> D{tag_d}
    //
    //
    // After:
    //
    //   A{tag_a}               B{tag_b}
    //       \                    /
    //   E{tag_c, tag_d, tag_e}  /
    //          \               /
    //           D{tag_c, tag_d}
    //
    // Comment:
    //   * D is the replacement root replacing C and creating a new argument node E
    //
    {
        auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
        auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

        auto a = make_shared<op::v1::Add>(x, y);
        a->add_provenance_tag("tag_a");
        auto b = make_shared<op::v1::Multiply>(y, x);
        b->add_provenance_tag("tag_b");
        auto c = make_shared<op::v1::Subtract>(a, b);
        c->add_provenance_tag("tag_c");

        auto f = make_shared<Function>(c, ParameterVector{x, y});

        auto e = make_shared<op::v1::Subtract>(a, x);
        e->add_provenance_tag("tag_e");
        auto d = make_shared<op::v1::Subtract>(e, b);
        d->add_provenance_tag("tag_d");

        replace_node(c, d);

        EXPECT_EQ(d->get_provenance_tags(), (ProvSet{"tag_c", "tag_d"}));
        EXPECT_EQ(e->get_provenance_tags(), (ProvSet{"tag_c", "tag_e"}));
    }
}

TEST(provenance, add_group_above) {
    auto p1 = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
    p1->add_provenance_tag("P1");
    auto p2 = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
    p2->add_provenance_tag("P2");
    auto a1 = make_shared<op::v1::Add>(p1, p2);
    auto m1 = make_shared<op::v1::Multiply>(a1, a1)->add_provenance_group_members_above({p1, p2});
    m1->add_provenance_tag("m1");
    EXPECT_EQ(p1->get_provenance_tags(), (ProvSet{"P1"}));
    EXPECT_EQ(p2->get_provenance_tags(), (ProvSet{"P2"}));
    EXPECT_EQ(a1->get_provenance_tags(), (ProvSet{"m1"}));
    EXPECT_EQ(m1->get_provenance_tags(), (ProvSet{"m1"}));
}

TEST(provenance, add_tags_above) {
    auto x = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
    auto y = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});

    auto a = make_shared<op::v1::Add>(x, y);
    auto b = make_shared<op::v1::Multiply>(x, y);
    auto c = make_shared<op::v1::Subtract>(a, b);
    auto d = make_shared<op::Abs>(c);

    // Add tags to Subtract and all nodes until Parameters (all above c, until params x, y)
    c->add_provenance_tags_above(OutputVector{x, y}, {"tag_above_c - until_params"});
    // Add tags to Abs and Subtract (above d, until c inputs)
    d->add_provenance_tags_above(c->input_values(), {"tag_above_d - until_c_inputs"});
    // Add tags to Abs and all nodes above
    d->add_provenance_tags_above(OutputVector{}, {"tag_all_above_d"});

    auto x_tags = x->get_provenance_tags();
    EXPECT_EQ(x_tags.size(), 1);
    EXPECT_TRUE(x_tags.find("tag_all_above_d") != x_tags.end());

    auto y_tags = y->get_provenance_tags();
    EXPECT_EQ(y_tags.size(), 1);
    EXPECT_TRUE(y_tags.find("tag_all_above_d") != y_tags.end());

    auto a_tags = a->get_provenance_tags();
    EXPECT_EQ(a_tags.size(), 2);
    EXPECT_TRUE(a_tags.find("tag_above_c - until_params") != a_tags.end());
    EXPECT_FALSE(a_tags.find("tag_above_d - until_c_inputs") != a_tags.end());
    EXPECT_TRUE(a_tags.find("tag_all_above_d") != a_tags.end());

    auto b_tags = b->get_provenance_tags();
    EXPECT_EQ(b_tags.size(), 2);
    EXPECT_TRUE(b_tags.find("tag_above_c - until_params") != b_tags.end());
    EXPECT_FALSE(b_tags.find("tag_above_d - until_c_inputs") != b_tags.end());
    EXPECT_TRUE(b_tags.find("tag_all_above_d") != b_tags.end());

    auto c_tags = c->get_provenance_tags();
    EXPECT_EQ(c_tags.size(), 3);
    EXPECT_TRUE(c_tags.find("tag_above_c - until_params") != c_tags.end());
    EXPECT_TRUE(c_tags.find("tag_above_d - until_c_inputs") != c_tags.end());
    EXPECT_TRUE(c_tags.find("tag_all_above_d") != c_tags.end());

    auto d_tags = d->get_provenance_tags();
    EXPECT_EQ(d_tags.size(), 2);
    EXPECT_FALSE(d_tags.find("tag_above_c - until_params") != d_tags.end());
    EXPECT_TRUE(d_tags.find("tag_above_d - until_c_inputs") != d_tags.end());
    EXPECT_TRUE(d_tags.find("tag_all_above_d") != d_tags.end());
}

TEST(provenance, builder) {
    auto p1 = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
    p1->add_provenance_tag("P1");
    auto norm = builder::opset1::lp_norm(p1, op::Constant::create(element::i64, {}, {0}), 1, 0);
    norm->add_provenance_tag("norm");
    for (auto node : topological_sort(NodeVector{norm})) {
        if (node == p1) {
            EXPECT_EQ(node->get_provenance_tags(), (ProvSet{"P1"}));
        } else {
            EXPECT_EQ(node->get_provenance_tags(), (ProvSet{"norm"}));
        }
    }
}

TEST(provenance, empty_group) {
    auto p1 = make_shared<op::Parameter>(element::i32, PartialShape{2, 3, 4});
    p1->add_provenance_tag("P1");
    auto abs = make_shared<op::Abs>(p1);
    // Make sure group is empty
    abs->add_provenance_group_members_above({abs});
    abs->add_provenance_tag("abs");
    for (auto node : topological_sort(NodeVector{abs})) {
        if (node == p1) {
            EXPECT_EQ(node->get_provenance_tags(), (ProvSet{"P1"}));
        } else {
            EXPECT_EQ(node->get_provenance_tags(), (ProvSet{"abs"}));
        }
    }
}
