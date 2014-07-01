/**
 * @file
 * Provide the Chunk object.
 */

(function(globals, $){

  'use strict';

  /**
   * Manages properties and methods of a single chunk within a ChunkField.
   *
   * @prop {HTMLElement} element
   *   The DOM element which this Chunk ought to be tied to.
   * @prop {ChunksField} field
   *   The parent ChunkField object to which this Chunk belongs.
   * @prop {integer} delta
   *   The position of this chunk in relation to other chunks in this field.
   *   This is tied to the delta for the field value in Drupal's Field API.
   */
  globals.Chunk = function(element, field, delta) {

    // Make it easier to reference this Chunk when scope changes.
    var thisChunk = this;


    /**
     * Public properties.
     */

    this.element = $(element);
    this.field = field;
    this.delta = delta;
    this.classPrepend = '.' + field.classFieldName + '-' + delta + '-';
    this.namePrepend =  field.fieldName + '[' + field.langcode + '][' + delta + ']';
    this.chunkInstance = $(':input[name="' + this.namePrepend + '[instance]"]:checked').val();
    this.chunkType = $(':input[name="' + this.namePrepend + '[type]"]').val();
    this.viewElement = $(':input[name="' + this.namePrepend + '[view]"]');
    this.view = this.viewElement.val();
    this.active = $(element).hasClass('active');
    this.addButton = $(':input[name="' + field.fieldName + '-' + delta + '-add-after"]');
    this.viewWrappers = {
      'instance_selection': $('> .form-item-' + field.classFieldName + '-' + field.langcode + '-' + this.delta + '-instance', this.element),
      'configuration': $(this.classPrepend + 'configuration', this.element),
      'preview': $(this.classPrepend + 'preview', this.element),
    };
    this.conditionalButtons = $('.chunk-edit-button, .chunk-preview-button, .chunk-cancel-button', this.element);

    /**
     * Public methods.
     */

    this.setAsActive = function() {
      this.element.addClass('active');
      this.active = true;
    };

    this.setAsUnactive = function() {
      this.element.removeClass('active');
      this.active = false;
    };

    this.getConfigState = function() {
      var configuration = {};
      this.chunkInstance = $(':input[name="' + this.namePrepend + '[instance]"]:checked').val();
      this.chunkType = Drupal.settings.chunks[field.fieldName].instances[this.chunkInstance].type;

      $('[name^="' + this.namePrepend + '[configuration][' + this.chunkInstance + ']"]').each(function(i, element) {
        var name = $(element).attr('name');
        var configProp = name.match(/[^\[]*(?=]$)/)[0];
        configuration[configProp] = $(element).val();
      });

      return configuration;
    };

    this.saveConfig = function() {
      this.chunkInstance = $(':input[name="' + this.namePrepend + '[instance]"]:checked').val();
      this.chunkType = Drupal.settings.chunks[field.fieldName].instances[this.chunkInstance].type;

      $('[name^="' + this.namePrepend + '[configuration][' + this.chunkInstance + ']"]').each(function(i, element) {
        var name = $(element).attr('name');
        var configProp = name.match(/[^\[]*(?=]$)/)[0];
        Drupal.settings.chunks[field.fieldName].chunks[delta].configuration[thisChunk.chunkInstance][configProp] = $(element).val();
      });

      // Provide a callback with which to perform actions against the saved
      // configuration.
      if (typeof Drupal.settings.chunks.callbacks.saveConfig[this.chunkType] === 'function') {
        Drupal.settings.chunks.callbacks.saveConfig[this.chunkType](field.fieldName, field.langcode, delta);
      }
    };

    this.restoreConfig = function() {
      this.chunkInstance = $(':input[name="' + this.namePrepend + '[instance]"]:checked').val();
      this.chunkType = Drupal.settings.chunks[field.fieldName].instances[this.chunkInstance].type;

      $('[name^="' + this.namePrepend + '[configuration]"]').each(function(i, element) {
        var name = $(element).attr('name');
        var configProp = name.match(/[^\[]*(?=]$)/)[0];
        $(element).val(Drupal.settings.chunks[field.fieldName].chunks[delta].configuration[thisChunk.chunkInstance][configProp]);
      });

      // Provide a callback with which to perform actions against the
      // restored configuration.
      if (typeof Drupal.settings.chunks.callbacks.restoreConfig[this.chunkType] === 'function') {
        Drupal.settings.chunks.callbacks.restoreConfig[this.chunkType](field.fieldName, field.langcode, delta);
      }
    };

    this.setView = function(view) {
      this.view = view;
      this.viewElement.val(view);
      this.viewElement.trigger('change');

      // Hide all conditionally visible buttons.
      this.conditionalButtons.hide();

      switch (view) {
        case 'staged':
          // Hide staged chunks from the parent row.
          this.element.parents('#' + field.fieldName + '-' + this.delta + '-chunk-row').hide();
          break;

        case 'instance_selection':
          break;

        case 'configuration':
          // Save configuration so it can be restored later if the edit is
          // cancelled.
          this.saveConfig();

          // Hide unused configuration forms.
          if (typeof this.chunkInstance !== 'undefined') {
            $('.fieldset-wrapper > div', this.viewWrappers.configuration).hide();
            $('.' + this.chunkInstance + '-chunk-instance-configuration', this.viewWrappers.configuration).show();
          }

          // Show preview and cancel buttons.
          this.conditionalButtons.filter('.chunk-cancel-button, .chunk-preview-button').show();
          break;

        case 'preview':
          // Show edit button.
          this.conditionalButtons.filter('.chunk-edit-button').show();
          break;
      }

      // Show wrapper element associated with the given view. Hide all others.
      for (var wrapper in this.viewWrappers) {
        if (wrapper === view) {
          this.viewWrappers[view].show();
          continue;
        }
        this.viewWrappers[wrapper].hide();
      }
    };

    // Remove active state from element by briefly removing it from the DOM.
    this.removeActiveElementState = function(el) {
      var p, s;
      p = el.parentNode;
      s = el.nextSibling;

      p.removeChild(el);
      p.insertBefore(el, s);
    };


    /**
     * Register some helpers.
     */

    // Navigate instance radio buttons.
    $(':input[name="' + this.namePrepend + '[instance]"]', element).bind('keydown.chunkInstanceNavigate', function(e) {
      if ((e.keyCode === 9 && !e.shiftKey) || e.keyCode === 39 || e.keyCode === 40) {
        // Navigate down.
        var nextRadio = $(this).parent().next('.form-item').children('.chunk-instance-selection');
        if (nextRadio.length > 0) {
          e.preventDefault();
          nextRadio.focus();
        }
      }
      else if ((e.keyCode === 9 && e.shiftKey) || e.keyCode === 37 || e.keyCode === 38) {
        // Navigate up.
        var prevRadio = $(this).parent().prev('.form-item').children('.chunk-instance-selection');
        if (prevRadio.length > 0) {
          e.preventDefault();
          prevRadio.focus();
        }
      }
    });

    // Instance selection.
    $(':input[name="' + this.namePrepend + '[instance]"]', element).bind('keyup.chunkInstanceSelected click.chunkInstanceSelected', function(e) {
      if (e.type === 'click' || e.type === 'keyup' && e.keyCode === 13) {

        // Make sure the proper radio is checked and the change event has
        // fired.
        this.checked = true;
        $(this).trigger('change');

        // Update chunkInstance.
        thisChunk.chunkInstance = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
        thisChunk.chunkType = Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].type;

        // Prepare form if the chunk instance is to be themed on the client.
        if (Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].settings.preview_on_client) {

          // Prevent default actions on preview button if we are theming on
          // the client.
          $(thisChunk.classPrepend + 'preview-button', element).unbind('click').unbind('keypress').bind('keypress', false);

          // Set module value since it may not be set in a form rebuild.
          $('[name^="' + thisChunk.namePrepend + '[module]"]').val(Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].chunk_type.module);
        }

        // Switch to configuration view.
        thisChunk.setView('configuration');

        // Set active class on the last chunk with user interaction.
        thisChunk.field.setActiveChunk(thisChunk.delta);

        // Add focus to first configuration item.
        setTimeout(function() {
          $('.' + thisChunk.chunkType + '-chunk-type-configuration :input:visible, [contenteditable]:visible', thisChunk.element).first().focus();
        }, 0);
      }
    });

    // Preview.
    $(this.classPrepend + 'preview-button', element).bind('keyup.chunkPreview mousedown.chunkPreview', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
        var preview;

        // We will trigger the click event manually if we want this button
        // to do anything outside this function.
        e.preventDefault();

        // Hide cancel button.
        $(thisChunk.classPrepend + 'cancel-button', element).hide();

        // Update chunkInstance and chunkType.
        thisChunk.chunkInstance = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
        thisChunk.chunkType = Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].type;

        // If we should be using a client-side theme implementation,
        // prevent the ajax call and build the preview.
        if (typeof thisChunk.chunkInstance !== 'undefined' && Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].settings.preview_on_client) {

          // Save configuration data.
          thisChunk.saveConfig();

          // Build preview.
          preview = Drupal.theme('chunk__' + thisChunk.chunkType, Drupal.settings.chunks[thisChunk.field.fieldName].chunks[thisChunk.delta].configuration[thisChunk.chunkInstance], thisChunk.field.fieldName, thisChunk.field.langcode, thisChunk.delta);
          $(thisChunk.classPrepend + 'preview')[0].innerHTML = preview;

          // Set focus to add chunk button.
          setTimeout(function() {
            thisChunk.addButton.focus();
          }, 0);

          // Remove active state on button.
          thisChunk.removeActiveElementState(this);
        }
        else {
          // Trigger click event so ajax call will fire.
          $(this).trigger('click');
        }

        // Switch to preview view.
        thisChunk.setView('preview');

        // Set active class on the last chunk with user interaction.
        thisChunk.field.setActiveChunk(thisChunk.delta);
      }
    });

    // Edit.
    $(this.classPrepend + 'edit-button', element).bind('keyup.chunkEdit mousedown.chunkEdit', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
        thisChunk.chunkInstance = $(':input[name="' + thisChunk.namePrepend + '[instance]"]:checked').val();
        thisChunk.chunkType = Drupal.settings.chunks[thisChunk.field.fieldName].instances[thisChunk.chunkInstance].type;

        // Show cancel button.
        $(thisChunk.classPrepend + 'cancel-button', element).show();

        // Switch to configuration view.
        thisChunk.setView('configuration');

        // Set active class on the last chunk with user interaction.
        thisChunk.field.setActiveChunk(thisChunk.delta);

        // Remove active state on button.
        thisChunk.removeActiveElementState(this);

        // Add focus to first configuration item.
        setTimeout(function() {
          $('.' + thisChunk.chunkType + '-chunk-type-configuration :input:visible, [contenteditable]:visible', thisChunk.element).first().focus();
        }, 0);
      }
    });

    // Cancel.
    $(this.classPrepend + 'cancel-button', element).bind('keyup.chunkEditCancel mousedown.chunkEditCancel', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

        // Hide cancel button.
        $(this).hide();

        // Switch to preview view.
        thisChunk.setView('preview');

        // Set active class on the last chunk with user interaction.
        thisChunk.field.setActiveChunk(thisChunk.delta);

        // Remove active state on button.
        thisChunk.removeActiveElementState(this);

        // Restore configuration.
        thisChunk.restoreConfig();

        // Set focus to add chunk button.
        setTimeout(function() {
          thisChunk.addButton.focus();
        }, 0);
      }
    });

    // Remove.
    $(this.classPrepend + 'remove-button', element).bind('keyup.chunkRemove mousedown.chunkRemove', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

        // Retrieve all currently visible chunks.
        var visibleChunks = $('.chunk-wrapper:visible', thisChunk.field.field);

        // Set focus to the add after button in the chunk before this one unless
        // this is the first visible chunk in which case we should set the
        // focus to the add before button.
        visibleChunks.each(function(vi, ve) {
          if (parseInt($(ve).attr('delta'), 10) === thisChunk.delta) {
            if (vi === 0) {
              setTimeout(function() {
                $(':input[name="' + thisChunk.field.fieldName + '-add-before"]', thisChunk.field.field).focus();
              }, 0);
            }
            else {
              setTimeout(function() {
                $(visibleChunks[vi - 1]).find('.add-chunk-action-after input').focus();
              }, 0);
            }
            return false;
          }
        });

        // Switch to removed view.
        thisChunk.setView('removed');

        // Remove javascript settings.
        delete Drupal.settings.chunks[thisChunk.field.fieldName].chunks[thisChunk.delta];

        // Hide the row for the removed chunk.
        $('#' + thisChunk.field.fieldName + '-' + thisChunk.delta + '-chunk-row').hide();

        // Reset odd/even striping on whole chunks field.
        thisChunk.field.resetStripes();
      }
    });

    // Add here (after)
    $(this.classPrepend + 'add-after-button', element).bind('keyup.chunkAdd mousedown.chunkAdd', function(e) {
      if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

        // Show the currently hidden staged chunk after the current one.
        thisChunk.field.showStagedChunk('#' + thisChunk.field.fieldName + '-' + thisChunk.delta + '-chunk-row');

        // Iterate over visible chunks to find the new chunk index which
        // indicates which chunk will receive focus when the field is
        // rebuilt. We cannot rely on deltas since they can change when
        // the field is reloaded (for instance, if there are chunks queued
        // for removal above this current chunk).
        $('.chunk-wrapper:visible', thisChunk.field.field).each(function(vi, ve) {
          if (parseInt($(ve).attr('delta'), 10) === thisChunk.delta) {
            Drupal.settings.chunks[thisChunk.field.fieldName].newChunkIndex = vi + 1;
            return false;
          }
        });
      }
    });

    // The edit, cancel, and remove buttons should never reload the page
    // if javascript is enabled.
    $(this.classPrepend + 'edit-button, ' + this.classPrepend + 'cancel-button, ' + this.classPrepend + 'remove-button.unlimited', this.element).click(function(e) {
      e.preventDefault();
    });


    /**
     * Perform initial operations.
     */

    this.setView(this.view);

    // If this chunk was just added, focus the type selection form
    // element.
    if (delta === Drupal.settings.chunks[field.fieldName].newChunkIndex) {
      // Set focus.
      $(':input[name="' + this.namePrepend + '[instance]"]', element).first().focus();
      // Reset index until we add another new chunk.
      Drupal.settings.chunks[field.fieldName].newChunkIndex = undefined;
    }

    // Switch to configuration view and save configuration when errors are detected.
    if ($(this.classPrepend + 'configuration .error', element).length > 0) {
      this.setView('configuration');
      $(':input.error', element).first().focus();
    }
    // If no errors were detected, set the focus to the active chunk's add
    // button.
    else if (this.active) {
      this.addButton.focus();
    }

    // Allow other modules to perform actions on a chunk upon it's
    // initialization.
    if (this.chunkType === '') {
      // Run the callback for every type if the chunk does not currently have
      // one assigned.
      for (var chunkType in Drupal.settings.chunks.callbacks.initialize) {
        if (typeof Drupal.settings.chunks.callbacks.initialize[chunkType] === 'function') {
          Drupal.settings.chunks.callbacks.initialize[chunkType](this);
        }
      }
    }
    else {
      // Run the callback only for the type assigned to the chunk.
      if (typeof Drupal.settings.chunks.callbacks.initialize[this.chunkType] === 'function') {
        Drupal.settings.chunks.callbacks.initialize[this.chunkType](this);
      }
    }
  };
})(this, jQuery);
