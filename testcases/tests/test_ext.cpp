#include "../../core/ext/sparsehash/sparsehash.h"
#include "test.h"

void rt::UnitTests::sparsehash()
{
	ext::fast_set<int>	set;
	set.insert(1);
	set.insert(2);
	set.insert(3);
	set.insert(4);
	set.insert(5);
	set.insert(6);
	set.insert(7);
	set.insert(8);
	set.insert(9);
	set.insert(10);

	for(auto it: set)
		if((it & 1) == 0)
			set.erase(it);

	rt::String out;
	for(auto it: set)
		out += rt::SS(" ") + it;

	_LOG(out);
}