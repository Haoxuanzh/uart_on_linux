# uart_on_linux
an unfinished app to send and receive message through uart on the platform of linux
This program is used to send and receive messages through the uart port in linux os. I use the primary way in which I send and receive message by using write() and read function. 
However, there seems some problem to my code because when I open the excutable file, then type into the information that I want to send(I connected the txd to rxd of the same uart port), it keeps sneding( the sneding led and the receiving led keeps being enlightened which indicates the txd is sneding messages), while the stings I typed in are usually short which do not require such a long period to send. 
