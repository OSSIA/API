#include "device.hpp"
#include "model.hpp"
#include "parameter.hpp"
#include "remote.hpp"
#include "view.hpp"

namespace ossia { namespace pd {

static t_eclass *device_class;

static void device_register(t_device* x){
    x->x_node->clearChildren();
    x->register_children();
}

static void *device_new(t_symbol *name, int argc, t_atom *argv)
{
    t_device *x = (t_device *)eobj_new(device_class);
    t_binbuf* d = binbuf_via_atoms(argc,argv);

    if(x && d)
    {
        x->x_name = gensym("Pd");
        // NOTE Don't know why this is not set by the CICM default setter
        x->x_protocol = gensym("Minuit");
        x->x_remoteip = gensym("127.0.0.1");
        x->x_localport = 9998;
        x->x_remoteport = 13579;

        x->x_dumpout = outlet_new((t_object*)x,gensym("dumpout"));

        if (argc != 0 && argv[0].a_type == A_SYMBOL) {
            x->x_name = atom_getsymbol(argv);
        }

        auto local_proto_ptr = std::make_unique<ossia::net::local_protocol>();
        ossia::net::local_protocol& local_proto = *local_proto_ptr;
        x->x_device = new ossia::net::generic_device{std::move(local_proto_ptr), x->x_name->s_name};
        x->x_node = &x->x_device->getRootNode();
        x->x_device->onAddressCreated.connect<t_device, &t_device::addressCreationHandler>(x);

        ebox_attrprocess_viabinbuf(x, d);

        try {
            if(x->x_protocol == gensym("Minuit")){
                local_proto.exposeTo(std::make_unique<ossia::net::minuit_protocol>(x->x_name->s_name, x->x_remoteip->s_name, x->x_remoteport, x->x_localport));
                logpost(x,3,"connect to %s on port %d, listening on port %d",  x->x_remoteip->s_name, x->x_remoteport, x->x_localport);
            } else {
                pd_error((t_object*)x, "Unknown protocol: %s", x->x_protocol->s_name);
            }
        } catch (ossia::connection_error e) {
            pd_error(x,"%s",e.what());
        }
    }

    return (x);
}

static void device_free(t_device *x)
{
    x->x_dead = true;
    x->unregister_children();
    if (x->x_device) delete(x->x_device);
}

static void dump_child(t_device *x, const ossia::net::node_base& node){
    for (const auto& child : node.children()){
        std::stringstream ss;
        auto parent = child->getParent();
        while (parent != nullptr)
        {
          ss << "\t";
          parent = parent->getParent();
        }
        ss << child->getName();
        t_atom a;
        SETSYMBOL(&a,gensym(ss.str().c_str()));
        outlet_anything(x->x_dumpout, gensym("child"), 1, &a);
        dump_child(x, *child);
    }
}

static void device_dump(t_device *x){
    dump_child(x, x->x_device->getRootNode());
}

void t_device :: register_children(){

    // first register parameters
    std::vector<obj_hierachy> params = find_child(x_obj.o_canvas->gl_list, osym_param, 0);
    std::sort(params.begin(), params.end());
    for (auto v : params){
        t_param* param = (t_param*) v.x;
        param->register_node(this->x_node);
    }

    // then register model, this could register some parameter again
    std::vector<obj_hierachy> models = find_child(x_obj.o_canvas->gl_list, osym_model, 0);
    std::sort(models.begin(), models.end());
    for (auto v : models){
        t_model* model = (t_model*) v.x;
        model->register_node(this->x_node);
    }

    // then register remote
    std::vector<obj_hierachy> remotes = find_child(x_obj.o_canvas->gl_list, osym_remote, 0);
    std::sort(remotes.begin(), remotes.end());
    for (auto v : remotes){
        t_remote* remote = (t_remote*) v.x;
        remote->register_node(this->x_node);
    }

    std::vector<obj_hierachy> views = find_child(x_obj.o_canvas->gl_list, osym_view, 0);
    std::sort(views.begin(), views.end());
    for (auto v : views){
        t_view* view = (t_view*) v.x;
        view->register_node(this->x_node);
    }
}

void t_device :: unregister_children(){
    // unregister in the reverse order to unregister parameter and remote before model and view
    // now they are connected to aboutToBeDeleted signal and thus no need to unregister
    /*
    std::vector<obj_hierachy> remotes = find_child(x_obj.o_canvas->gl_list, osym_remote, 0);
    std::sort(remotes.begin(), remotes.end());
    for (auto v : remotes){
        t_remote* remote = (t_remote*) v.x;
        remote->unregister();
    }

    std::vector<obj_hierachy> models = find_child(x_obj.o_canvas->gl_list, osym_model, 0);
    std::sort(models.begin(), models.end());
    for (auto v : models){
        t_model* model = (t_model*) v.x;
        model->unregister();
    }

    std::vector<obj_hierachy> params = find_child(x_obj.o_canvas->gl_list, osym_param, 0);
    std::sort(params.begin(), params.end());
    for (auto v : params){
        t_param* param = (t_param*) v.x;
        param->unregister();
    }

    std::vector<obj_hierachy> views = find_child(x_obj.o_canvas->gl_list, osym_view, 0);
    std::sort(views.begin(), views.end());
    for (auto v : views){
        t_view* view = (t_view*) v.x;
        view->unregister();
    }
    */
}

/*
void t_device :: addressCreationHandler(const ossia::net::address_base& n){
    for (auto model : t_model::quarantine()){
        obj_register<t_model>(static_cast<t_model*>(model));
    }

    for (auto param : t_param::quarantine()){
        obj_register<t_param>(static_cast<t_param*>(param));
    }

    for (auto view : t_view::quarantine()){
        obj_register<t_view>(static_cast<t_view*>(view));
    }

    for (auto remote : t_remote::quarantine()){
        obj_register<t_remote>(static_cast<t_remote*>(remote));
    }
}
*/

/*
// FIXME is this really necessary ?
void t_device :: nodeCreationHandler(const ossia::net::node_base& n){
    for (auto view : t_view::quarantine()){
        obj_register<t_view>(static_cast<t_view*>(view));
    }
}
*/

static void device_expose(t_device* x, t_symbol*, int argc, t_atom* argv){
    if (argc && argv->a_type == A_SYMBOL){
        if (argv->a_w.w_symbol == gensym("Minuit")){
            // TODO how to add protocol to actual device ?
            // remote_ip remote_port local_port
        }
    }
}

extern "C" void setup_ossia0x2edevice(void)
{
    t_eclass *c = eclass_new("ossia.device", (method)device_new, (method)device_free, (short)sizeof(t_device), CLASS_DEFAULT, A_GIMME, 0);

    if(c)
    {
        eclass_addmethod(c, (method) device_register, "register", A_NULL, 0);
        eclass_addmethod(c, (method) device_dump, "dump", A_NULL, 0);
        eclass_addmethod(c, (method) device_expose, "expose", A_GIMME, 0);

        CLASS_ATTR_SYMBOL (c, "protocol",   0, t_device, x_protocol);
        CLASS_ATTR_SYMBOL (c, "remoteip",   0, t_device, x_remoteip);
        CLASS_ATTR_INT    (c, "localport",  0, t_device, x_localport);
        CLASS_ATTR_INT    (c, "remoteport", 0, t_device, x_remoteport);

        CLASS_ATTR_DEFAULT(c, "protocol",   0, "Minuit");
        CLASS_ATTR_DEFAULT(c, "remoteip",   0, "127.0.0.1");
        CLASS_ATTR_DEFAULT(c, "localport",  0, "9998");
        CLASS_ATTR_DEFAULT(c, "remoteport", 0, "13579");
    }

    device_class = c;
}


} } //namespace
