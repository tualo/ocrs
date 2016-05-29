#ocrs

This program read scanned letter images. It regonizes the barcode, the address
text and if possible it find the sort box. It support customer depending sort
box execptions. So it is possible to block some zip codes for particular
customers. The customers will be defined by the file name. If the filename
contains the letter N the textpart before will be used as customer
identification.

This program needs a database for optaining the sort informations. It's
recommend to use a local database for that. The local database may use
connected tables (federated or connction engine).
