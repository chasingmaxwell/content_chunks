/**
 * @file
 * Add some extra reactivity to the chunks field widget.
 */

(function($){

  'use strict';

  var removeActiveState, newChunkIndex;

  // Remove active state from element by briefly removing it from the DOM.
  removeActiveState = function(el) {
    var p, s;
    p = el.parentNode;
    s = el.nextSibling;

    p.removeChild(el);
    p.insertBefore(el, s);
  };

  Drupal.behaviors.chunksWidget = {
    attach: function(context, settings) {

      // Perform actions on each chunk field.
      $('.chunks-field', context).each(function() {
        var field, fieldName, classFieldName, langcode, chunks, config, setActiveChunk, showStagedChunk, resetStripes, saveConfig, restoreConfig;

        /**
         * Set variables applicable to each chunks field.
         */
        field = $(this);
        fieldName = field.attr('field_name');
        classFieldName = field.attr('field_name').replace(/_/g, '-');
        langcode = field.attr('langcode');
        chunks = $('.chunk-wrapper', field);
        config = {};

        /**
         * Provide helper functions that operate on the current chunks field.
         */

        // Set "active" class on a specific chunk.
        setActiveChunk = function(chunk) {
          chunks.removeClass('active');
          chunk.addClass('active');
        };

        // Show a staged chunk.
        showStagedChunk = function(prevSibling) {
          var stagedRow, stagedChunk, stagedDelta, stagedViewElement;
          stagedRow = $('tr.staged', field);
          stagedChunk = $('.chunk-wrapper', stagedRow);
          stagedDelta = parseInt(stagedChunk.attr('delta'), 10);
          stagedViewElement = $(':input[name="' + fieldName + '[' + langcode + '][' + stagedDelta + '][view]"]');
          stagedViewElement.val('type_selection');
          stagedViewElement.trigger('change');
          stagedRow.removeClass('staged');
          stagedRow.insertAfter(prevSibling);
          stagedRow.show();
          resetStripes();
        };

        // Reset odd/even striping on visible chunk rows.
        resetStripes = function() {
          var visibleChunks = $('.chunk-wrapper:visible', field);
          visibleChunks.each(function(i, element) {
            var delta, parentRow;

            delta = parseInt($(element).attr('delta'), 10);
            parentRow = $('#' + fieldName + '-' + delta + '-chunk-row');

            if (((i + 1) % 2) == 1) {
              parentRow.removeClass('even').addClass('odd');
            }
            else {
              parentRow.removeClass('odd').addClass('even');
            }
          });
        };

        // Save configuration for chunk with given delta.
        saveConfig = function(delta) {
          var chunkType = $(':input[name="' + fieldName + '[' + langcode + '][' + delta + '][type]"]:checked').val();

          config[delta] = {};

          $('[name^="' + fieldName + '[' + langcode + '][' + delta + '][configuration][' + chunkType + ']"]').each(function(i, element) {
            var name = $(element).attr('name');
            config[delta][name] = $(element).val();
          });
        };

        // Restore configuration for chunk with given delta.
        restoreConfig = function(delta) {
          $('[name^="' + fieldName + '[' + langcode + '][' + delta + '][configuration]"]').each(function(i, element) {
            var name = $(element).attr('name');
             $(element).val(config[delta][name]);
          });

          delete config[delta];
        };

        /**
         * Register event handlers relative to each chunk field.
         */

        // Add here (before)
        $(':input[name="' + fieldName + '-add-before"]', field).bind('keyup.chunkAdd mousedown.chunkAdd', function(e) {
          if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

            // Show the currently hidden staged chunk above every other chunk.
            showStagedChunk('#' + fieldName + '-chunks-field .add-chunk-action-before-row');

            // Set the newChunkIndex to 0 so we can properly focus it when the
            // field is rebuilt.
            newChunkIndex = 0;
          }
        });

        /**
         * Perform actions on each chunk.
         */
        chunks.each(function(index, element) {
          var delta, classPrepend, namePrepend, chunkType, viewElement, view, active, addButton;

          /**
           * Set variables applicable to each individual chunk.
           */
          delta = parseInt($(element).attr('delta'), 10);
          classPrepend = '.' + classFieldName + '-' + delta + '-';
          namePrepend =  fieldName + '[' + langcode + '][' + delta + ']';
          chunkType = $(':input[name="' + namePrepend + '[type]"]:checked').val();
          viewElement = $(':input[name="' + namePrepend + '[view]"]');
          view = viewElement.val();
          active = $(element).hasClass('active');
          addButton = $(':input[name="' + fieldName + '-' + delta + '-add-after"]');

          /**
           * Perform initial actions on each chunk.
           */

          // If this chunk is staged, hide it.
          if (view == 'staged') {
            $(element).parents('#' + fieldName + '-' + delta + '-chunk-row').hide();
          }

          // If this chunk is being viewed in configuration view initially, save
          // the configuration so that the "cancel" button will revert changes.
          if (view == 'configuration') {
            saveConfig(delta);
          }

          // If there is already a selected type and that type is configured to
          // be themed on the client, prevent default actions on the preview
          // button.
          if (typeof chunkType !== 'undefined' && Drupal.settings.chunks[chunkType].instance_type_settings.preview_on_client) {
            // Prevent default actions on preview button if we are theming on
            // the client.
            $(classPrepend + 'preview-button', element).unbind('click').unbind('keypress').bind('keypress', false);
          }

          // If this chunk was just added, focus the type selection form
          // element.
          if (delta === newChunkIndex) {
            // Set focus.
            $(':input[name="' + namePrepend + '[type]"]', element).first().focus();
            // Reset index until we add another new chunk.
            newChunkIndex = undefined;
          }

          // Switch to configuration view and save configuration when errors are detected.
          if ($(classPrepend + 'configuration .error', element).length > 0) {
            viewElement.val('configuration');
            viewElement.trigger('change');
            $(':input.error', element).first().focus();
            saveConfig(delta);
          }
          // If no errors were detected, set the focus to the active chunk's add
          // button.
          else if (active) {
            addButton.focus();
          }

          /**
           * Register event handlers relative to each chunk.
           */

          // Navigate types.
          $(':input[name="' + namePrepend + '[type]"]', element).bind('keydown.chunkTypeNavigate', function(e) {
            if ((e.keyCode === 9 && !e.shiftKey) || e.keyCode === 39 || e.keyCode === 40) {
              // Navigate down.
              var nextRadio = $(this).parent().next('.form-item').children('.chunk-type-selection');
              if (nextRadio.length > 0) {
                e.preventDefault();
                nextRadio.focus();
              }
            }
            else if ((e.keyCode === 9 && e.shiftKey) || e.keyCode === 37 || e.keyCode === 38) {
              // Navigate up.
              var prevRadio = $(this).parent().prev('.form-item').children('.chunk-type-selection');
              if (prevRadio.length > 0) {
                e.preventDefault();
                prevRadio.focus();
              }
            }
          });

          // Type selection.
          $(':input[name="' + namePrepend + '[type]"]', element).bind('keyup.chunkTypeSelected click.chunkTypeSelected', function(e) {
            if (e.type === 'click' || e.type === 'keyup' && e.keyCode === 13) {

              // Make sure the proper radio is checked and the change event has
              // fired.
              this.checked = true;
              $(this).trigger('change');

              // Update chunkType.
              chunkType = $(':input[name="' + namePrepend + '[type]"]:checked').val();

              // Prepare form if the chunk type is to be themed on the client.
              if (Drupal.settings.chunks[chunkType].instance_type_settings.preview_on_client) {

                // Prevent default actions on preview button if we are theming on
                // the client.
                $(classPrepend + 'preview-button', element).unbind('click').unbind('keypress').bind('keypress', false);

                // Set module value since it may not be set in a form rebuild.
                $('[name^="' + namePrepend + '[module]"]').val(Drupal.settings.chunks[chunkType].module);
              }

              // Switch to configuration view.
              viewElement.val('configuration');
              viewElement.trigger('change');

              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));

              // Add focus to first configuration item.
              setTimeout(function() {
                $('[name^="' + namePrepend + '[configuration][' + chunkType + ']"]').first().focus();
              }, 0);
            }
          });

          // Preview.
          $(classPrepend + 'preview-button', element).bind('keyup.chunkPreview mousedown.chunkPreview', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              var configuration, newProp, preview;

              // We will trigger the click event manually if we want this button
              // to do anything outside this function.
              e.preventDefault();

              // Hide cancel button.
              $(classPrepend + 'cancel-button', element).hide();

              // Update chunkType.
              chunkType = $(':input[name="' + namePrepend + '[type]"]:checked').val();

              // If we should be using a client-side theme implementation,
              // prevent the ajax call and build the preview.
              if (typeof chunkType !== 'undefined' && Drupal.settings.chunks[chunkType].instance_type_settings.preview_on_client) {

                // Save configuration data.
                saveConfig(delta);

                // Parse configuration data.
                configuration = {};
                for (var name in config[delta]) {
                  newProp = name.match(/[^\[]*(?=]$)/)[0];
                  configuration[newProp] = config[delta][name];
                }

                // Build preview.
                preview = Drupal.theme('chunk__' + chunkType, configuration);
                $(classPrepend + 'preview').html(preview);

                // Remove configuration data.
                delete config[delta];

                // Set focus to add chunk button.
                setTimeout(function() {
                  addButton.focus();
                }, 0);

                // Remove active state on button.
                removeActiveState(this);
              }
              else {
                // Trigger click event so ajax call will fire.
                $(this).trigger('click');
              }

              // Switch to preview view.
              viewElement.val('preview');
              viewElement.trigger('change');

              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
            }
          });

          // Edit.
          $(classPrepend + 'edit-button', element).bind('keyup.chunkEdit mousedown.chunkEdit', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              chunkType = $(':input[name="' + namePrepend + '[type]"]:checked').val();

              // Show cancel button.
              $(classPrepend + 'cancel-button', element).show();

              // Switch to configuration view.
              viewElement.val('configuration');
              viewElement.trigger('change');

              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));

              // Remove active state on button.
              removeActiveState(this);

              // Save configuration so we can restore it if we cancel.
              saveConfig(delta);

              // Add focus to first configuration item.
              setTimeout(function() {
                $('[name^="' + namePrepend + '[configuration][' + chunkType + ']"]').first().focus();
              }, 0);
            }
          });

          // Cancel.
          $(classPrepend + 'cancel-button', element).bind('keyup.chunkEditCancel mousedown.chunkEditCancel', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

              // Hide cancel button.
              $(this).hide();

              // Switch to preview view.
              viewElement.val('preview');
              viewElement.trigger('change');

              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));

              // Remove active state on button.
              removeActiveState(this);

              // Restore configuration.
              restoreConfig(delta);

              // Set focus to add chunk button.
              setTimeout(function() {
                addButton.focus();
              }, 0);
            }
          });

          // Remove.
          $(classPrepend + 'remove-button', element).bind('keyup.chunkRemove mousedown.chunkRemove', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

              // Retrieve all currently visible chunks.
              var visibleChunks = $('.chunk-wrapper:visible', field);

              // Set focus to the add after button in the chunk before this one unless
              // this is the first visible chunk in which case we should set the
              // focus to the add before button.
              visibleChunks.each(function(vi, ve) {
                if ($(ve).attr('delta') === $(element).attr('delta')) {
                  if (vi === 0) {
                    setTimeout(function() {
                      $(':input[name="' + fieldName + '-add-before"]', field).focus();
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
              viewElement.val('removed');
              viewElement.trigger('change');

              // Hide the row for the removed chunk.
              $('#' + fieldName + '-' + delta + '-chunk-row').hide();

              // Reset odd/even striping on whole chunks field.
              resetStripes();
            }
          });

          // Add here (after)
          $(classPrepend + 'add-after-button', element).bind('keyup.chunkAdd mousedown.chunkAdd', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {

              // Show the currently hidden staged chunk after the current one.
              showStagedChunk('#' + fieldName + '-' + delta + '-chunk-row');

              // Iterate over visible chunks to find the new chunk index which
              // indicates which chunk will receive focus when the field is
              // rebuilt. We cannot rely on deltas since they can change when
              // the field is reloaded (for instance, if there are chunks queued
              // for removal above this current chunk).
              $('.chunk-wrapper:visible', field).each(function(vi, ve) {
                if ($(ve).attr('delta') === $(element).attr('delta')) {
                  newChunkIndex = vi + 1;
                  return false;
                }
              });
            }
          });

          // The edit, cancel, and remove buttons should never reload the page
          // if javascript is enabled.
          $(classPrepend + 'edit-button, ' + classPrepend + 'cancel-button, ' + classPrepend + 'remove-button', element).click(function(e) {
            e.preventDefault();
          });
        });
      });
    }
  };

})(jQuery);
