-- ALTER TABLE WebVisu.DataBindings ADD CONSTRAINT `DataBindings_OPCUAMirrorNodes_FK_1` FOREIGN KEY (`srcOPCUANodeID`) REFERENCES `OPCUAMirrorNodes` (`ID`) ON DELETE CASCADE;
-- SHOW CREATE TABLE DataBindings;
-- OPCUAMirrorNodesIdentifierTypes
USE WebVisu;
-- CREATE TABLE IF NOT EXISTS `OPCUAMirrorNodesIdentifierTypes` (
--   `ID` char(1) NOT NULL,
--   `IdentifierType` varchar(100) DEFAULT NULL,
--   `description` varchar(100) DEFAULT NULL,
--   PRIMARY KEY (`ID`)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- OPCUAMirrorNodes
-- CREATE TABLE IF NOT EXISTS `OPCUAMirrorNodes` (
--   `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
--   `namespaceIndex` int(10) unsigned NOT NULL,
--   `IdentifierTypeID` char(1) NOT NULL,
--   `identifier` varchar(100) NOT NULL,
--   `dataType` varchar(100) NOT NULL,
--   PRIMARY KEY (`ID`),
--   KEY `OPCUAMirrorNodes_OPCUAMirrorNodesIdentifierTypes_FK` (`IdentifierTypeID`),
--   CONSTRAINT `OPCUAMirrorNodes_OPCUAMirrorNodesIdentifierTypes_FK` FOREIGN KEY (`IdentifierTypeID`) REFERENCES `OPCUAMirrorNodesIdentifierTypes` (`ID`)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- Pages
CREATE TABLE IF NOT EXISTS `Pages` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ParentID` bigint(20) unsigned DEFAULT NULL,
  `title` varchar(30) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `Pages_Pages_FK` (`ParentID`),
  CONSTRAINT `Pages_Pages_FK` FOREIGN KEY (`ParentID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsTypes
CREATE TABLE IF NOT EXISTS `GuiElementsTypes` (
  `ID` int(10) unsigned NOT NULL,
  `type` varchar(32) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElements
CREATE TABLE IF NOT EXISTS `GuiElements` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `PageID` bigint(20) unsigned DEFAULT NULL,
  `TypeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElements_Pages_FK` (`PageID`),
  KEY `GuiElements_GuiElementsTypes_FK` (`TypeID`),
  CONSTRAINT `GuiElements_GuiElementsTypes_FK` FOREIGN KEY (`TypeID`) REFERENCES `GuiElementsTypes` (`ID`),
  CONSTRAINT `GuiElements_Pages_FK` FOREIGN KEY (`PageID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsDataNodesTemplate
CREATE TABLE `GuiElementsDataNodesTemplate` (
  `ID` int(10) unsigned NOT NULL,
  `type` varchar(100) NOT NULL,
  `defaultValue` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesDataNodesRel
CREATE TABLE `GuiElementTypesDataNodesRel` (
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `DataNodeTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`DataNodeTemplateID`),
  KEY `GuiElementTypesDataNodesRel_FK_1` (`DataNodeTemplateID`),
  CONSTRAINT `GuiElementTypesDataNodesRel_FK` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementsTypes` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesDataNodesRel_FK_1` FOREIGN KEY (`DataNodeTemplateID`) REFERENCES `GuiElementsDataNodesTemplate` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsDataNodes
CREATE TABLE `GuiElementsDataNodes` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsDataNodes_GuiElements_FK` (`GuiElementID`),
  KEY `GuiElementsDataNodes_FK` (`typeID`),
  CONSTRAINT `GuiElementsDataNodes_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementsDataNodesTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementsDataNodes_GuiElements_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsParamsTemplate
CREATE TABLE `GuiElementsParamsTemplate` (
  `ID` int(10) unsigned NOT NULL,
  `type` varchar(100) NOT NULL,
  `defaultValue` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesParamRel
CREATE TABLE `GuiElementTypesParamRel` (
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `ParamTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`ParamTemplateID`),
  KEY `GuiElementTypesParamRel_FK` (`ParamTemplateID`),
  CONSTRAINT `GuiElementTypesParamRel_FK` FOREIGN KEY (`ParamTemplateID`) REFERENCES `GuiElementsParamsTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesParamRel_FK_1` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementsTypes` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementsParams
CREATE TABLE `GuiElementsParams` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementsParams_GuiElements_FK` (`GuiElementID`),
  KEY `GuiElementsParams_FK` (`typeID`),
  CONSTRAINT `GuiElementsParams_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementsParamsTemplate` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementsParams_GuiElements_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- Databindings
-- CREATE TABLE IF NOT EXISTS `DataBindings` (
--   `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
--   `destGuiElementDataNodeID` bigint(20) unsigned DEFAULT NULL,
--   `srcGuiElementDataNodeID` bigint(20) unsigned DEFAULT NULL,
--   `destOPCUANodeID` bigint(20) unsigned DEFAULT NULL,
--   `srcOPCUANodeID` bigint(20) unsigned DEFAULT NULL,
--   PRIMARY KEY (`ID`),
--   KEY `DataBindings_OPCUAMirrorNodes_FK` (`destOPCUANodeID`),
--   KEY `DataBindings_OPCUAMirrorNodes_FK_1` (`srcOPCUANodeID`),
--   KEY `DataBindings_GuiELementsDataNodes_FK` (`destGuiElementDataNodeID`),
--   KEY `DataBindings_GuiELementsDataNodes_FK_1` (`srcGuiElementDataNodeID`),
--   CONSTRAINT `DataBindings_OPCUAMirrorNodes_FK` FOREIGN KEY (`destOPCUANodeID`) REFERENCES `OPCUAMirrorNodes` (`ID`) ON DELETE CASCADE,
--   CONSTRAINT `DataBindings_OPCUAMirrorNodes_FK_1` FOREIGN KEY (`srcOPCUANodeID`) REFERENCES `OPCUAMirrorNodes` (`ID`) ON DELETE CASCADE,
--   CONSTRAINT `DataBindings_GuiELementsDataNodes_FK` FOREIGN KEY (`destGuiElementDataNodeID`) REFERENCES `GuiElementsDataNodes` (`ID`) ON DELETE CASCADE,
--   CONSTRAINT `DataBindings_GuiELementsDataNodes_FK_1` FOREIGN KEY (`srcGuiElementDataNodeID`) REFERENCES `GuiElementsDataNodes` (`ID`) ON DELETE CASCADE,
--   CONSTRAINT `justOneBindingPerDatarecord` CHECK (`destGuiElementDataNodeID` is not null and `srcGuiElementDataNodeID` is null and `destOPCUANodeID` is null and `srcOPCUANodeID` is not null or `destGuiElementDataNodeID` is null and `srcGuiElementDataNodeID` is not null and `destOPCUANodeID` is not null and `srcOPCUANodeID` is null or `destGuiElementDataNodeID` is not null and `srcGuiElementDataNodeID` is not null and `destOPCUANodeID` is null and `srcOPCUANodeID` is null)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- INSERT ENUMERATION VALUES
-- DELETE FROM OPCUAMirrorNodesIdentifierTypes WHERE ID='b' or ID='g' or ID='i' or ID = 's';
-- INSERT INTO OPCUAMirrorNodesIdentifierTypes (ID, IdentifierType, description) Values
-- 	('b', 'OPAQUE (ByteString)', NULL),
-- 	('g', 'GUID (Guid)', 'global unique...'),
-- 	('i', 'NUMERIC (UInteger)', NULL),
-- 	('s', 'STRING (String)', NULL);
DELETE FROM GuiElementsTypes WHERE ID= 1 or ID = 2;
INSERT INTO GuiElementsTypes (ID, `type`) VALUES
	(1, 'button'),
	(2, 'p');
Insert INTO Pages (title) values
	('sampleTitle'),('anotherSampleTitle');
