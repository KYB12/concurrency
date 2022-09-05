Implementing multithreading to an existing web server.<br/>
Code was pulled from https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/concurrency-webserver.<br />
I simply edited wclient.c to allow the server to handle multiple requests at once via multithreading, given the functionality of the server. <br/>
Additionally, to test, I edited wclient.c in order to make sure the server could actually handle multiple requests at once.<br/>
Usage: ./wclient hostname port number_of_requests <br/>

