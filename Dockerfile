FROM brainboxdotcc/dpp:latest

WORKDIR /usr/src/marong

COPY . .

WORKDIR /usr/src/marong/build

RUN cmake ..
RUN make -j$(nproc)

ENTRYPOINT [ "/usr/src/marong/build/marong" ]
