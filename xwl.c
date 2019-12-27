#include "labwc.h"

int xwl_nr_parents(struct tinywl_view *view) {
	struct wlr_xwayland_surface *s = view->xwayland_surface;
	int i = 0;

	if (!s) {
		fprintf(stderr, "warn: (%s) no xwayland surface\n", __func__);
		return -1;
	}
	while (s->parent) {
		s = s->parent;
		++i;
	}
	return i;
}

void xwl_surface_map(struct wl_listener *listener, void *data) {
	struct tinywl_view *view = wl_container_of(listener, view, map);
	view->mapped = true;
	view->been_mapped = true;
	view->x = view->xwayland_surface->x;
	view->y = view->xwayland_surface->y;
	view->surface = view->xwayland_surface->surface;
	focus_view(view, view->xwayland_surface->surface);
}

void xwl_surface_unmap(struct wl_listener *listener, void *data) {
	struct tinywl_view *view = wl_container_of(listener, view, unmap);
	view->mapped = false;
	if (is_toplevel(view))
		view_focus_next_toplevel(view->server);
}

void xwl_surface_destroy(struct wl_listener *listener, void *data) {
	struct tinywl_view *view = wl_container_of(listener, view, destroy);
	wl_list_remove(&view->link);
	free(view);
}

void xwl_surface_configure(struct wl_listener *listener, void *data) {
	struct tinywl_view *view = wl_container_of(listener, view, request_configure);
	struct wlr_xwayland_surface_configure_event *event = data;
	wlr_xwayland_surface_configure(view->xwayland_surface, event->x, event->y,
		event->width, event->height);
}

void xwl_surface_new(struct wl_listener *listener, void *data) {
	struct tinywl_server *server =
		wl_container_of(listener, server, new_xwayland_surface);
	struct wlr_xwayland_surface *xwayland_surface = data;
	wlr_xwayland_surface_ping(xwayland_surface);

	struct tinywl_view *view = calloc(1, sizeof(struct tinywl_view));
	view->server = server;
	view->type = LAB_XWAYLAND_VIEW;
	view->xwayland_surface = xwayland_surface;

	view->map.notify = xwl_surface_map;
	wl_signal_add(&xwayland_surface->events.map, &view->map);
	view->unmap.notify = xwl_surface_unmap;
	wl_signal_add(&xwayland_surface->events.unmap, &view->unmap);
	view->destroy.notify = xwl_surface_destroy;
	wl_signal_add(&xwayland_surface->events.destroy, &view->destroy);
	view->request_configure.notify = xwl_surface_configure;
	wl_signal_add(&xwayland_surface->events.request_configure, &view->request_configure);

	wl_list_insert(&server->views, &view->link);
}
