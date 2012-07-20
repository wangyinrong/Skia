
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkClipStack.h"
#include "SkPath.h"
#include "SkRect.h"



static void test_assign_and_comparison(skiatest::Reporter* reporter) {
    SkClipStack s;
    bool doAA = false;

    REPORTER_ASSERT(reporter, 0 == s.getSaveCount());

    // Build up a clip stack with a path, an empty clip, and a rect.
    s.save();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());

    SkPath p;
    p.moveTo(5, 6);
    p.lineTo(7, 8);
    p.lineTo(5, 9);
    p.close();
    s.clipDevPath(p, SkRegion::kIntersect_Op, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    SkRect r = SkRect::MakeLTRB(1, 2, 3, 4);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);
    r = SkRect::MakeLTRB(10, 11, 12, 13);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);

    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);

    // Test that assignment works.
    SkClipStack copy = s;
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different save levels triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    REPORTER_ASSERT(reporter, s != copy);

    // Test that an equal, but not copied version is equal.
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);
    REPORTER_ASSERT(reporter, s == copy);

    // Test that a different op on one level triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(14, 15, 16, 17);
    s.clipDevRect(r, SkRegion::kIntersect_Op, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Test that different state (clip type) triggers not equal.
    // NO LONGER VALID: if a path contains only a rect, we turn
    // it into a bare rect for performance reasons (working
    // around Chromium/JavaScript bad pattern).
/*
    s.restore();
    s.save();
    SkPath rp;
    rp.addRect(r);
    s.clipDevPath(rp, SkRegion::kUnion_Op, doAA);
    REPORTER_ASSERT(reporter, s != copy);
*/

    // Test that different rects triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 3 == s.getSaveCount());

    r = SkRect::MakeLTRB(24, 25, 26, 27);
    s.clipDevRect(r, SkRegion::kUnion_Op, doAA);
    REPORTER_ASSERT(reporter, s != copy);

    // Sanity check
    s.restore();
    REPORTER_ASSERT(reporter, 2 == s.getSaveCount());

    copy.restore();
    REPORTER_ASSERT(reporter, 2 == copy.getSaveCount());
    REPORTER_ASSERT(reporter, s == copy);
    s.restore();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());
    copy.restore();
    REPORTER_ASSERT(reporter, 1 == copy.getSaveCount());
    REPORTER_ASSERT(reporter, s == copy);

    // Test that different paths triggers not equal.
    s.restore();
    REPORTER_ASSERT(reporter, 0 == s.getSaveCount());
    s.save();
    REPORTER_ASSERT(reporter, 1 == s.getSaveCount());

    p.addRect(r);
    s.clipDevPath(p, SkRegion::kIntersect_Op, doAA);
    REPORTER_ASSERT(reporter, s != copy);
}

static void assert_count(skiatest::Reporter* reporter, const SkClipStack& stack,
                         int count) {
    SkClipStack::B2TIter iter(stack);
    int counter = 0;
    while (iter.next()) {
        counter += 1;
    }
    REPORTER_ASSERT(reporter, count == counter);
}

static void test_iterators(skiatest::Reporter* reporter) {
    SkClipStack stack;

    static const SkRect gRects[] = {
        { 0,   0,  40,  40 },
        { 60,  0, 100,  40 },
        { 0,  60,  40, 100 },
        { 60, 60, 100, 100 }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRects); i++) {
        // the union op will prevent these from being fused together
        stack.clipDevRect(gRects[i], SkRegion::kUnion_Op, false);
    }

    assert_count(reporter, stack, 4);

    // bottom to top iteration
    {
        const SkClipStack::B2TIter::Clip* clip = NULL;

        SkClipStack::B2TIter iter(stack);
        int i;

        for (i = 0, clip = iter.next(); clip; ++i, clip = iter.next()) {
            REPORTER_ASSERT(reporter, *clip->fRect == gRects[i]);
        }

        SkASSERT(i == 4);
    }

    // top to bottom iteration
    {
        const SkClipStack::Iter::Clip* clip = NULL;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
        int i;

        for (i = 3, clip = iter.prev(); clip; --i, clip = iter.prev()) {
            REPORTER_ASSERT(reporter, *clip->fRect == gRects[i]);
        }

        SkASSERT(i == -1);
    }

    // skipToTopmost
    {
        const SkClipStack::Iter::Clip*clip = NULL;

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kBottom_IterStart);

        clip = iter.skipToTopmost(SkRegion::kUnion_Op);
        REPORTER_ASSERT(reporter, *clip->fRect == gRects[3]);
    }
}

static void TestClipStack(skiatest::Reporter* reporter) {
    SkClipStack stack;

    REPORTER_ASSERT(reporter, 0 == stack.getSaveCount());
    assert_count(reporter, stack, 0);

    static const SkIRect gRects[] = {
        { 0, 0, 100, 100 },
        { 25, 25, 125, 125 },
        { 0, 0, 1000, 1000 },
        { 0, 0, 75, 75 }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRects); i++) {
        stack.clipDevRect(gRects[i], SkRegion::kIntersect_Op);
    }

    // all of the above rects should have been intersected, leaving only 1 rect
    SkClipStack::B2TIter iter(stack);
    const SkClipStack::B2TIter::Clip* clip = iter.next();
    SkRect answer;
    answer.iset(25, 25, 75, 75);

    REPORTER_ASSERT(reporter, clip);
    REPORTER_ASSERT(reporter, clip->fRect);
    REPORTER_ASSERT(reporter, !clip->fPath);
    REPORTER_ASSERT(reporter, SkRegion::kIntersect_Op == clip->fOp);
    REPORTER_ASSERT(reporter, *clip->fRect == answer);
    // now check that we only had one in our iterator
    REPORTER_ASSERT(reporter, !iter.next());

    stack.reset();
    REPORTER_ASSERT(reporter, 0 == stack.getSaveCount());
    assert_count(reporter, stack, 0);

    test_assign_and_comparison(reporter);
    test_iterators(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ClipStack", TestClipStackClass, TestClipStack)
