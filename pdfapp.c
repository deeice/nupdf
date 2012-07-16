#include "fitz.h"
#include "mupdf.h"
#include "pdfapp.h"


static void pdfapp_warn(pdfapp_t *app, const char *fmt, ...)
{
	fprintf(stderr, "this warning function was eaten by a grue,"
					"but feel better knowing that something"
					"went wrong, and it wasn't your fault\n");
}

static void pdfapp_error(pdfapp_t *app, fz_error error)
{
	fprintf(stderr, "this error module was eaten by a grue,"
					"but feel better knowing that something"
					"went wrong, and it wasn't your fault\n");
}


void pdfapp_init(pdfapp_t *app)
{
	fz_error error;
	memset(app, 0, sizeof(pdfapp_t));
	error = fz_newrenderer(&app->rast, pdf_devicergb, 0, 1024 * 512);
	if (error)
		pdfapp_error(app, error);
}

void pdfapp_open(pdfapp_t *app, char *filename)
{
	fz_error error;
	fz_obj *obj;
	char *password = "";

	/*
	 * Open PDF and load xref table
	 */

	app->filename = filename;

	app->xref = pdf_newxref();
	error = pdf_loadxref(app->xref, filename);
	if (error)
	{
		fz_catch(error, "trying to repair");
		error = pdf_repairxref(app->xref, filename);
		if (error)
			pdfapp_error(app, error);
	}

	error = pdf_decryptxref(app->xref);
	if (error)
		pdfapp_error(app, error);

	/*
	 * Handle encrypted PDF files
	 */

	if (pdf_needspassword(app->xref))
	{
		int okay = pdf_authenticatepassword(app->xref, password);
		while (!okay)
		{	
			if (!password)
				exit(1);
			okay = pdf_authenticatepassword(app->xref, password);
			if (!okay)
				pdfapp_warn(app, "Invalid password.");
		}
	}

	/*
	 * Load meta information
	 * TODO: move this into mupdf library
	 */

	obj = fz_dictgets(app->xref->trailer, "Root");
	app->xref->root = fz_resolveindirect(obj);
	if (!app->xref->root)
		pdfapp_error(app, fz_throw("syntaxerror: missing Root object"));
	fz_keepobj(app->xref->root);

	obj = fz_dictgets(app->xref->trailer, "Info");
	app->xref->info = fz_resolveindirect(obj);
	if (!app->xref->info)
		pdfapp_warn(app, "Could not load PDF meta information.");
	if (app->xref->info)
		fz_keepobj(app->xref->info);

	/*app->outline = pdf_loadoutline(app->xref);*/

	app->doctitle = filename;
	if (strrchr(app->doctitle, '\\'))
		app->doctitle = strrchr(app->doctitle, '\\') + 1;
	if (strrchr(app->doctitle, '/'))
		app->doctitle = strrchr(app->doctitle, '/') + 1;
	if (app->xref->info)
	{
		obj = fz_dictgets(app->xref->info, "Title");
		if (obj)
		{
			app->doctitle = pdf_toutf8(obj);
		}
	}

	/*
	 * Start at first page
	 */

	app->pagecount = pdf_getpagecount(app->xref);

	
	app->rotate = 0;
	
}

void pdfapp_close(pdfapp_t *app)
{
	if (app->page)
		pdf_droppage(app->page);
	app->page = nil;

	if (app->image)
		fz_droppixmap(app->image);
	app->image = nil;

/*	if (app->outline)
		pdf_dropoutline(app->outline);
	app->outline = nil;*/

	if (app->xref->store)
		pdf_dropstore(app->xref->store);
	app->xref->store = nil;

	pdf_closexref(app->xref);
	app->xref = nil;
}



