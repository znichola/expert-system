# Use to build a deply the project
FROM alpine:3.20

RUN apk add --no-cache \
    clang \
    make \
    graphviz-dev \
    graphviz \
    libstdc++

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Set environment variables so the Makefile finds Graphviz
ENV GV_PREFIX=/usr
ENV WITH_GRAPHVIZ=1

# Build the project
RUN make all WITH_GRAPHVIZ=1 GV_INCS="-DWITH_GRAPHVIZ -I/usr/include/graphviz" GV_LIBS="-L/usr/lib -lgvc -lcgraph -lcdt -Wl,-rpath,/usr/lib"

# Expose the server port
EXPOSE 7711

# Default command (start the app)
ENTRYPOINT ["./expert-system", "--server", "--port=7711"]

# Build the image
# docker build -t znichola/expert-system:latest .

# Test locally
# docker run -p 7711:7711 znichola/expert-system

# Push to Docker Hub
# docker push znichola/expert-system:latest