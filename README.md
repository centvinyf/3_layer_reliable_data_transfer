# 3_layer_reliable_data_transfer

This is a simulation of 3-layer reliable data transfer(physical layer, datalink layer, applicationlayer).

To start the programe, using ./project2 *0/1 **0-100 ***0-100 ****0/1

*=1 if the endsystem is a server, 0 if it's a client.
** represents for the data drop rate
*** represents for the data corruption rate
****=1 if the RDT protocol is GBN, =0 if the RDT protocol is Selective_repeat.

COMMANDS:
LIST: List all the names of the files in the dir (who can be supported for transfer)
SEND: send a file to the other side.

Now we can support send files from both side and receive them correctly from the otherside.

To compile the project, just use "make" command
