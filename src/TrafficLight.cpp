#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    //DONE
    
    std::cout << " MessageQueue receive: thread id = " << std::this_thread::get_id() << std::endl;
    std::unique_lock<std::mutex> uLock(_mutex);

    _condition.wait(uLock, [this] {
        return !_queue.empty();
    });

    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    //DONE
    
    //set the gaurd on the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    //add message to queue
    _queue.push_back(std::move(msg));
    
    //notify state
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    //set current state to red light
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    //DONE
    
    //continue this loopuntil nothing recieved
    while (true) {
        
        //test for green
        if (_queue->receive() == TrafficLightPhase::green) {
            return;
        }
        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    //added the scope if needed??
    return TrafficLight::_currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    //DONE
    
    //set the function cycleThroughPhases to start on its thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    //DONE 
    
    //grab the last updated time
    std::chrono::time_point<std::chrono::system_clock> lastTimeUpdated = std::chrono::system_clock::now();
    
    //***************************************
    // random number design structure from https://www.guyrutenberg.com/2014/05/03/c-mt19937-example/
    
    //random number device
    std::random_device randomNum;
    //use pseudo-random number generator
    std::mt19937 eng(randomNum());
    
    //cycle duration is a random value between 4 and 6 seconds
    std::uniform_int_distribution<> uDistr(4000, 6000);
    
    //set the duration
    int cycleDur = uDistr(eng);
    
    //***************************************
    
    //stay in loop until false
    while (true) {
        
        //sleep for 1 millisecond, tried longer with strange results
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        //calc the current span of time since last update
        int currentTimeSpanSinceUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastTimeUpdated).count();

        //tes if we are over the cycle duration
        if (currentTimeSpanSinceUpdate >= cycleDur) {
            
            //grab the cycle duration
            cycleDur = uDistr(eng);
            
            //set the last updated time to now
            lastTimeUpdated = std::chrono::system_clock::now();
            
            //if red turn to green and vice versa, toggle update
            if (_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;

            //set current phase data to the message queue using move semantics
            std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _queue, std::move(_currentPhase));
        }
    }
}
