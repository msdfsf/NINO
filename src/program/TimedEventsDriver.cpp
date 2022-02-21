#pragma once

#include "TimedEventsDriver.h"

#include <chrono>
#include <thread>
#include "Windows.h"

#define millis() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
//#define millis() std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::duration<std::chrono::milliseconds>)std::chrono::high_resolution_clock::now()).count();
namespace TimedEventsDriver {

	int run = 1;

	int updatePeriod = 1;

	void main();
	std::thread thread(main);

	Node* firstNode = NULL;
	Node* lastNode = firstNode;

	void main() {
		while (run) {

			// sleep a bit, to not fuck a lot cpu
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			Node* node = firstNode;
			while (node != NULL) {

				const unsigned int time = millis();
				if (time - node->lastUpdate >= node->period) {
					node->lastUpdate = time;
					node->callback(node->ctrl);
				}
				
				node = node->nextNode;

			}


		}
	}

	void add(Control* ctrl, void (*callback) (Control* ctrl), int period) {
		
		Node* const newNode = (Node*) malloc(sizeof(Node));
		newNode->nextNode = NULL;
		newNode->prevNode = NULL;
		newNode->period = period;
		newNode->ctrl = ctrl;
		newNode->callback = callback;
		newNode->period = period;

		if (lastNode == NULL) {
			firstNode = newNode;
			lastNode = newNode;
		} else {
			lastNode->nextNode = newNode;
		}

		lastNode = newNode;

	}

	void terminate() {

		run = 0;
		thread.join();

	}
}
