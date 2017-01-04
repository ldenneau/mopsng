echo "Running simple test:";
./findtracklets file fake_small2.txt eval true | grep "Exact Match:" | \
   awk '{if ($9 == "1.000000," && $13 == "1.000000") { print "PASS" } else { print "FAIL" } }';
echo "Running elongation test:";
./findtracklets file testcase.dets use_pht true greedy true eval true | grep "Exact Match:" | \
   awk '{if ($9 == "1.000000," && $13 == "1.000000") { print "PASS" } else { print "FAIL" } }';

  