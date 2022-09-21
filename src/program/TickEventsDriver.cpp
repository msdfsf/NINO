#pragma once

#include "TickEventsDriver.h"

#include <chrono>
#include <thread>
#include "Render.h"
#include "Windows.h"

#define millis() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
//#define millis() std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::duration<std::chrono::milliseconds>)std::chrono::high_resolution_clock::now()).count();
namespace TickEventsDriver {

	int run = 1;

	int tickRate = 30;
	int tickDuration = 1000 / tickRate; // in ms

	void main();
	std::thread thread(main);

	Node* firstNode = NULL;
	Node* lastNode = firstNode;

	void main() {
		int cnt = 0;
		while (run) {

			const unsigned int startTime = millis();

			int isAnythingToRender = 0;

			Node* node = firstNode;
			while (node != NULL) {

				if (node->idleSignal) 
					if (!*((int*) node->idleSignal)) {
						node = node->nextNode;
						continue;
					}

				node->ticksPassed++;

				if (node->period >= node->ticksPassed + 1) {
					node = node->nextNode;
					continue;
				}

				if (node->type == TET_RENDER) isAnythingToRender = 1;
				
				if (node->callback)
					if (node->callback(node->data)) remove(node);

				node->ticksPassed = 0;
				node = node->nextNode;

			}

			if (isAnythingToRender) Render::redraw();

			const unsigned int endTime = millis();
			const unsigned int deltaTime = endTime - startTime;
			if (deltaTime <= tickDuration) {
				// tickDuration - deltaTime
				std::this_thread::sleep_for(std::chrono::milliseconds(tickDuration - deltaTime));
			} else {
				// is it any good to try compensate lost time?
			}

			cnt++;

		}

	}

	 Node* add(void* data, int (*callback) (void* ctrl), int period) {
		
		Node* const newNode = (Node*) malloc(sizeof(Node));
		if (!newNode) return NULL;

		newNode->nextNode = NULL;
		newNode->prevNode = NULL;
		newNode->period = period;
		newNode->data = data;
		newNode->callback = callback;
		newNode->period = period;
		newNode->ticksPassed = 0;
		newNode->idleSignal = NULL;

		if (lastNode == NULL) {
			firstNode = newNode;
			lastNode = newNode;
		} else {
			lastNode->nextNode = newNode;
		}

		lastNode = newNode;

		return newNode;
	 
	 }

	 void remove(Node* node) {

		 if (!node->prevNode) {
			 firstNode = node->nextNode;
		 } else {
			 node->prevNode = node->nextNode;
		 }

	 }

	void setTickRate(int tickRate) {

		tickDuration = 1000 / (tickRate < 0 ? 0 : tickRate);
	
	}

	void terminate() {

		run = 0;
		thread.join();

	}
}
