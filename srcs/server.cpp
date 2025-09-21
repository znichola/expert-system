    #include <graphviz/gvc.h>

    #include <iostream>

    void launchGraphvisServer() {
        // std::cout << "Launding grpahvis server!\n" << std::endl;
 
        std::string dotSpec = "strict digraph {A -> B\nB -> C }";

        GVC_t *gvc = gvContext();

        Agraph_t *g = agmemread(dotSpec.c_str());
        if (!g) {
            std::cerr << "Error: could not parse graph spec.\n";
            gvFreeContext(gvc);
            return;
        }

        gvLayout(gvc, g, "dot");

        gvRender(gvc, g, "png", stdout);

        gvFreeLayout(gvc, g);

        agclose(g);

        gvFreeContext(gvc);
    }
