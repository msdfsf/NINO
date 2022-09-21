#pragma once

#include "Control.h"

namespace TickEventsDriver {

	extern int tickRate; // ticks per second

	typedef enum EventType {

		TET_BASIC,
		TET_RENDER

	};

	typedef struct Node Node;
	struct Node {

		Node* nextNode;
		Node* prevNode;

		void* data;
		int (*callback) (void* data) = NULL;
		
		int period; // in ticks
		int ticksPassed; // to track

		void* idleSignal; // if pointed value is NULL, then skips node

		EventType type;

	};

	Node* add(void* data, int (*callback) (void* ctrl), int period);
	void remove(Node* node);

	void terminate();

}
